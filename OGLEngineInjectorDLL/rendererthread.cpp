#include "rendererthread.h"
#include "oglutils.h"
#include "scopedoglcontext.h"
#include <utils_misc.h>
#include "injectedenginerunner.h"


RendererThread::RendererThread()
	: m_flipWallpaperTextures(false),
	m_firstChange(true),
	m_fading(false)
{
	InitMonitors();
	CheckMonitorShift();
}

bool RendererThread::Init()
{
	m_messageEvent.Reset(CreateEvent(NULL, FALSE, FALSE, NULL));
	m_lastFlushTime = util::GetTimeMilis();

	HWND worker;
	HWND defView;
	HWND listView;

	if(!util::FindDesktopWindows(worker, defView, listView))
	{
		return false;
	}

	m_window = listView;
	if(GetWindowLongPtr(m_window, GWLP_USERDATA) != 0)
	{
		// SFML will overwrite the userdata, which it is not supposed to do.
		return false;
	}

	RECT clientRect;
	GetClientRect(m_window, &clientRect);
	m_width = clientRect.right - clientRect.left;
	m_height = clientRect.bottom - clientRect.top;

	m_oglWindow.create(m_window, sf::ContextSettings(24, 8));
	bool creationOk = m_oglWindow.isOpen();
	m_oldUserData = GetWindowLongPtr(m_window, GWLP_USERDATA);

	{
		ScopedOGLContext<sf::Window> soglc(m_oglWindow);

		oglutil::InitOpenGLOrthographic(m_width, m_height, false);
		m_frontWallpaperTexture = oglutil::CreateRGB8Texture(m_width, m_height, false);
		m_backWallpaperTexture = oglutil::CreateRGB8Texture(m_width, m_height, false);
	}

	m_thread.reset(new sf::Thread(&RendererThread::ThreadProc, this));

	return true;
}

void RendererThread::Start()
{
	m_thread->launch();
}

void RendererThread::Stop()
{
	QueueMessage(Message::MessageType::Close);

	m_thread->wait();
}

void RendererThread::Deinit()
{
	{
		ScopedOGLContext<sf::Window> soglc(m_oglWindow);
		glDeleteTextures(1, &m_frontWallpaperTexture);
		glDeleteTextures(1, &m_backWallpaperTexture);
	}

	m_oglWindow.close();

	LONG_PTR currentUserData = GetWindowLongPtr(m_window, GWLP_USERDATA);
	if(currentUserData != m_oldUserData)
	{
		util::dcout << "WTF, window userdata not set to SFML userdata !" << std::endl;
	}
	// Reset back the userdata to 0, which is the initial value.
	// Should have been done by SFML.
	SetWindowLongPtr(m_window, GWLP_USERDATA, 0);
}

void RendererThread::QueueUpdate()
{
	QueueMessage(Message::MessageType::Update);
}

void RendererThread::EmptyEventQueue()
{
	QueueMessage(Message::MessageType::EmptyEventQueue);
}


void RendererThread::SetIconsTexture(GLuint texture)
{
	m_iconsTexture = texture;
}

GLuint RendererThread::GetCurrentWallpaperTexture()
{
	bool currentFlip = m_flipWallpaperTextures.load();
	GLuint back = GetBackWallpaperTexture(currentFlip);
	FlipWallpaperTextures(currentFlip);

	if(!m_firstChange)
	{
		QueueMessage(Message::MessageType::FadeWallpaper);
	}
	else
	{
		m_firstChange = false;
	}

	return back;
}

void RendererThread::SetEngine(InjectedEngineRunner &engine, int monitorIndex)
{
	if(!IsMonitorIndexValid(monitorIndex))
	{
		return;
	}

	std::get<0>(m_engines[monitorIndex]) = &engine;
	std::get<1>(m_engines[monitorIndex]) = true;
}

void RendererThread::SetEnginePrimary(InjectedEngineRunner &engine)
{
	for (size_t i = 0; i < m_monitors.size(); i++)
	{
		if(m_monitors[i].IsPrimary())
		{
			SetEngine(engine, (int)i);
		}
	}
}

void RendererThread::ClearEngine(int monitorIndex)
{
	if(!IsMonitorIndexValid(monitorIndex))
	{
		return;
	}

	std::get<0>(m_engines[monitorIndex]) = NULL;
	std::get<1>(m_engines[monitorIndex]) = false;
}

void RendererThread::ClearEnginePrimary()
{
	for (size_t i = 0; i < m_monitors.size(); i++)
	{
		if(m_monitors[i].IsPrimary())
		{
			ClearEngine((int)i);
		}
	}
}

int RendererThread::GetMonitorWidth(int monitorIndex)
{
	if(!IsMonitorIndexValid(monitorIndex))
	{
		return -1;
	}

	return m_monitors[monitorIndex].GetRect().GetWidth();
}

int RendererThread::GetMonitorHeight(int monitorIndex)
{
	if(!IsMonitorIndexValid(monitorIndex))
	{
		return -1;
	}

	return m_monitors[monitorIndex].GetRect().GetHeight();
}

int RendererThread::GetPrimaryMonitorWidth()
{
	for (size_t i = 0; i < m_monitors.size(); i++)
	{
		if(m_monitors[i].IsPrimary())
		{
			return m_monitors[i].GetRect().GetWidth();
		}
	}

	return -1;
}

int RendererThread::GetPrimaryMonitorHeight()
{
	for (size_t i = 0; i < m_monitors.size(); i++)
	{
		if(m_monitors[i].IsPrimary())
		{
			return m_monitors[i].GetRect().GetWidth();
		}
	}

	return -1;
}

RendererThread::Message::Message()
	: Type(Message::Unspecified)
{
}

RendererThread::Message::Message(MessageType msgType)
	: Type(msgType)
{
}

void RendererThread::InitMonitors()
{
	m_monitors = util::GetDisplayMonitors();
	m_engines.resize(m_monitors.size());

	for (size_t i = 0; i < m_monitors.size(); i++)
	{
		std::get<0>(m_engines[i]) = NULL;
		std::get<1>(m_engines[i]) = false;
	}
}

void RendererThread::CheckMonitorShift()
{
	bool recomputeRects = false;
	for (size_t i = 0; i < m_monitors.size(); i++)
	{
		if(m_monitors[i].GetRect().GetX() < 0 ||
			m_monitors[i].GetRect().GetY() < 0)
		{
			recomputeRects = true;
		}
	}
	if(recomputeRects)
	{
		util::ShiftMonitorCoordinates(m_monitors);
	}
}

bool RendererThread::IsMonitorIndexValid(int monitorIndex)
{
	if(monitorIndex < 0 || monitorIndex >= m_monitors.size())
	{
		return false;
	}

	return true;
}

void RendererThread::ThreadProc()
{
	util::dcout << "Renderer thread entry @ " << GetCurrentThreadId() << std::endl;
	{
		// HACK: Activate and deactivate the context one time before sleeping
		//	     to avoid a crash under Catalyst 14.4 and allow SFML to
		//       create the internal context. If this is not done, when the
		//       context is deactivated, a crash will occur when SFML creates
		//       the internal context, specifically at wglMakeShared.
		ScopedOGLContext<sf::Window> soglc(m_oglWindow);
	}
	ScopedOGLContext<sf::Window> soglc(m_oglWindow);
	InitOpenGL();

	Message msg;
	m_quitLoop = false;
	while(!m_quitLoop)
	{
		WaitForSingleObject(m_messageEvent, !m_fading ? INFINITE : 16);

		if(!m_commandQueue.empty())
		{
			ProcessMessages();
		}

		if(m_fading)
		{
			auto currentTime = util::GetTimeMilis();
			auto delta = currentTime - m_lastFadeTime;
			m_currentFadeAmount += delta / 1000.0f;

			m_lastFadeTime = currentTime;
			if(m_currentFadeAmount > 1.0f)
			{
				m_fading = false;
			}

			DrawStep();
		}
	}
}

void RendererThread::ProcessMessages()
{
	Message msg;
	unsigned int dequeuedMsgCount = 0;

	while(m_commandQueue.tryDequeue(msg) && dequeuedMsgCount < 100)
	{
		switch (msg.Type)
		{
			case RendererThread::Message::Close:
			{
				m_quitLoop = true;
			
				return;
			}
			case RendererThread::Message::Update:
			{
				DrawStep();

				break;
			}
			case RendererThread::Message::EmptyEventQueue:
			{
				auto now = util::GetTimeMilis();
				if((now - m_lastFlushTime) > 5 * 1000)
				{
					m_lastFlushTime = now;

					sf::Event evt;
					while(m_oglWindow.pollEvent(evt))
					{
					}
				}

				break;
			}
			case RendererThread::Message::FadeWallpaper:
			{
				m_fading = true;
				m_currentFadeAmount = 0.0f;
				m_lastFadeTime = util::GetTimeMilis();

				break;
			}
			default:
			{
				break;
			}
		}

		dequeuedMsgCount++;
	}
}

void RendererThread::QueueMessage(Message::MessageType msgType)
{
	m_commandQueue.enqueue(Message(msgType));
	SetEvent(m_messageEvent);
}

void RendererThread::InitOpenGL()
{
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // non-premultiplied alpha
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // premultiplied alpha

	glColor3ub(255, 255, 255);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();
	glScalef(m_width, m_height, 0.0f);
}

void RendererThread::DrawStep()
{
	Draw();
	m_oglWindow.display();
}

void RendererThread::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawWallpaper();
	DrawEngines();
	DrawIcons();
}

void RendererThread::DrawWallpaper()
{
	GLuint currentFront;
	GLuint currentBack;

	GetFrontBackWallpaperTextures(currentFront, currentBack);

	if(m_fading)
	{
		glBindTexture(GL_TEXTURE_2D, currentBack);
		oglutil::DrawTexturedQuad();

		glPushAttrib(GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		float v = m_currentFadeAmount;
		glColor4f(v, v, v, v);
		glBindTexture(GL_TEXTURE_2D, currentFront);
		oglutil::DrawTexturedQuad();
		glPopAttrib();
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, currentFront);
		oglutil::DrawTexturedQuad();
	}
}

void RendererThread::DrawEngine(GLuint texture, util::Rectangle &rect)
{
	int x = rect.GetX();
	int y = rect.GetY();

	int w = rect.GetWidth();
	int h = rect.GetHeight();

	int iy = m_height - h - y;

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(x, iy, 0.0f);
	glScalef(w, h, 0.0f);

	glBindTexture(GL_TEXTURE_2D, texture);
	oglutil::DrawTexturedQuad();

	glPopMatrix();
}

void RendererThread::DrawEngines()
{
	for (size_t i = 0; i < m_engines.size(); i++)
	{
		if(!std::get<1>(m_engines[i]))
		{
			continue;
		}

		DrawEngine(std::get<0>(m_engines[i])->GetRenderTargetTexture(), m_monitors[i].GetRect());
	}
}

void RendererThread::DrawIcons()
{
	glBindTexture(GL_TEXTURE_2D, m_iconsTexture);
	oglutil::DrawTexturedQuad();
}

GLuint RendererThread::GetFrontWallpaperTexture()
{
	return GetFrontWallpaperTexture(m_flipWallpaperTextures);
}

GLuint RendererThread::GetFrontWallpaperTexture(bool flip)
{
	return flip ? m_backWallpaperTexture : m_frontWallpaperTexture;
}

GLuint RendererThread::GetBackWallpaperTexture()
{
	return GetBackWallpaperTexture(m_flipWallpaperTextures);
}

GLuint RendererThread::GetBackWallpaperTexture(bool flip)
{
	return flip ? m_frontWallpaperTexture : m_backWallpaperTexture;
}

void RendererThread::GetFrontBackWallpaperTextures(GLuint &front, GLuint &back)
{
	GetFrontBackWallpaperTextures(front, back, m_flipWallpaperTextures);
}

void RendererThread::GetFrontBackWallpaperTextures(GLuint &front, GLuint &back, bool flip)
{
	if(flip)
	{
		front = m_backWallpaperTexture;
		back = m_frontWallpaperTexture;
	}
	else
	{
		front = m_frontWallpaperTexture;
		back = m_backWallpaperTexture;
	}
}

void RendererThread::FlipWallpaperTextures()
{
	FlipWallpaperTextures(m_flipWallpaperTextures);
}

void RendererThread::FlipWallpaperTextures(bool currentFlip)
{
	// do atomic compare-exchange, should not fail
	// because only one thread, the wallpaper
	// hooker, writes to the variable.
	if(!m_flipWallpaperTextures.compare_exchange_weak(currentFlip, !currentFlip))
	{
		DebugBreak();
	}
}

