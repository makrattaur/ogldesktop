#include "wallpaperhook.h"
#include "oglutils.h"
#include <utils_win32.h>
#include <sstream>
#include "scopedoglcontext.h"


WallpaperHook::WallpaperHook(RendererThread &rendererThread)
	: m_rendererThread(rendererThread)
{
}

WallpaperHook::~WallpaperHook()
{
}

bool WallpaperHook::Hook()
{
	HookWindow(GetShellWindow());
	InitDrawHook(m_hookedWindow, false);

	DumpWindow();
	UpdateTexture(m_rendererThread.GetCurrentWallpaperTexture());

	return true;
}

bool WallpaperHook::Unhook()
{
	WindowHooker::UnhookWindow();
	DeInitDrawHook();

	return true;
}

LRESULT WallpaperHook::HookWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_PAINT:
		{
			LRESULT ret = CallOriginalWindowProc(msg, wParam, lParam);
			DumpWindow();

			UpdateTexture(m_rendererThread.GetCurrentWallpaperTexture());

			m_rendererThread.QueueUpdate();

			return ret;
		}
	}

	return WindowHooker::HookWndProc(msg, wParam, lParam);
}

void WallpaperHook::DumpWindow()
{
	HDC windowDC = GetWindowDC(m_hookedWindow);
	BitBlt(m_memoryDC->GetDC(), 0, 0, m_windowWidth, m_windowHeight, windowDC, 0, 0, SRCCOPY);
	ReleaseDC(m_hookedWindow, windowDC);
	GdiFlush();
}

void WallpaperHook::UpdateTexture(GLuint targetTexture)
{
	ScopedOGLContext<sf::Context> soglc(*m_oglContext);

	glBindTexture(GL_TEXTURE_2D, targetTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_windowWidth, m_windowHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, m_memoryDC->GetPixels());
}
