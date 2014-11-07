#include "injectedenginerunner.h"

#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <engine.h>
#include "scopedoglcontext.h"
#include "oglutils.h"


InjectedEngineRunner::InjectedEngineRunner(Engine *engine, RendererThread &rendererThread)
	: m_rendererThread(rendererThread),
	m_quit(false),
	m_engineContext(*this),
	m_flipTargets(false)
{
	SetEngineContext(m_engineContext);
	SetEngine(engine);
	SetTargetFrameRate(60);
}

InjectedEngineRunner::~InjectedEngineRunner()
{
}

bool InjectedEngineRunner::Init(int width, int height)
{
	m_width = width;
	m_height = height;

	{
		ScopedOGLContext<sf::Context> sgolc(m_context);
		m_glExt.Init();

		m_rt1.SetExtensions(m_glExt);
		m_rt1.Init(width, height);
		m_rt2.SetExtensions(m_glExt);
		m_rt2.Init(width, height);
	}

	m_thread.reset(new sf::Thread(&InjectedEngineRunner::ThreadProc, this));

	return true;
}

void InjectedEngineRunner::Start()
{
	m_thread->launch();
}

void InjectedEngineRunner::Stop()
{
	m_quit = true;
	m_thread->wait();
}

void InjectedEngineRunner::ProcessEvents(bool &needsToQuit)
{
	if(m_quit)
	{
		GetEngine()->SignalClose();
		needsToQuit = true;
	}
}

void InjectedEngineRunner::SwapBuffers()
{
	FlipBuffers();
}

GLuint InjectedEngineRunner::GetRenderTargetTexture()
{
	// present backbuffer
	return GetBackRenderTarget().GetTexture();

	//return GetFrontRenderTarget().m_targetTexture;
}

InjectedEngineRunner::CEngineContext::CEngineContext(InjectedEngineRunner &parent)
	: m_parent(parent)
{
}

InjectedEngineRunner::RenderTarget::RenderTarget()
	: m_destroyed(true),
	m_depthBuffer(0),
	m_targetTexture(0),
	m_frameBuffer(0)
{
}

void InjectedEngineRunner::RenderTarget::SetExtensions(GLExt &extensions)
{
	m_glExt = &extensions;
}

void InjectedEngineRunner::RenderTarget::Init(int width, int height)
{
	if(!m_destroyed)
	{
		DeInit();
	}

	//PFNGLGENFRAMEBUFFERSEXTPROC xx_glGenFramebuffersEXT = glGenFramebuffersEXT;

	m_glExt->m_glGenFramebuffersEXT(1, &m_frameBuffer);
	m_glExt->m_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frameBuffer);
	
	m_targetTexture = oglutil::CreateRGB8Texture(width, height, true);
	m_glExt->m_glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_targetTexture, 0);

	m_glExt->m_glGenRenderbuffersEXT(1, &m_depthBuffer);
	m_glExt->m_glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthBuffer);
	m_glExt->m_glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
	m_glExt->m_glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_depthBuffer);

	GLenum ret = m_glExt->m_glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(ret != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		DebugBreak();
	}

	m_destroyed = false;
}

void InjectedEngineRunner::RenderTarget::DeInit()
{
	if(m_destroyed)
	{
		return;
	}

	if(m_depthBuffer != 0)
	{
		m_glExt->m_glDeleteRenderbuffersEXT(1, &m_depthBuffer);
		m_depthBuffer = 0;
	}

	if(m_targetTexture != 0)
	{
		glDeleteTextures(1, &m_targetTexture);
		m_targetTexture = 0;
	}

	if(m_frameBuffer != 0)
	{
		m_glExt->m_glDeleteFramebuffersEXT(1, &m_frameBuffer);
		m_frameBuffer = 0;
	}

	m_destroyed = true;
}

void InjectedEngineRunner::RenderTarget::MakeCurrent()
{
	m_glExt->m_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frameBuffer);
}

GLuint InjectedEngineRunner::RenderTarget::GetTexture()
{
	return m_targetTexture;
}

void InjectedEngineRunner::RenderTarget::MakeDefaultFramebufferCurrent(GLExt *glExt)
{
	glExt->m_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

InjectedEngineRunner::GLExt::GLExt()
	: m_glGenFramebuffersEXT(NULL),
	m_glBindFramebufferEXT(NULL),
	m_glGenRenderbuffersEXT(NULL),
	m_glBindRenderbufferEXT(NULL),
	m_glRenderbufferStorageEXT(NULL),
	m_glFramebufferRenderbufferEXT(NULL),
	m_glFramebufferTexture2DEXT(NULL),
	m_glCheckFramebufferStatusEXT(NULL)
{
}

#define GET_WGL_FUNC(name) m_gl##name = (decltype(m_gl##name))wglGetProcAddress("gl" #name)
void InjectedEngineRunner::GLExt::Init()
{
	GET_WGL_FUNC(GenFramebuffersEXT);
	GET_WGL_FUNC(BindFramebufferEXT);
	GET_WGL_FUNC(GenRenderbuffersEXT);
	GET_WGL_FUNC(BindRenderbufferEXT);
	GET_WGL_FUNC(RenderbufferStorageEXT);
	GET_WGL_FUNC(FramebufferRenderbufferEXT);
	GET_WGL_FUNC(FramebufferTexture2DEXT);
	GET_WGL_FUNC(CheckFramebufferStatusEXT);
	GET_WGL_FUNC(DeleteRenderbuffersEXT);
	GET_WGL_FUNC(DeleteFramebuffersEXT);
}
#undef GET_WGL_FUNC

int InjectedEngineRunner::CEngineContext::GetWidth()
{
	return m_parent.m_width;
}

int InjectedEngineRunner::CEngineContext::GetHeight()
{
	return m_parent.m_height;
}

void InjectedEngineRunner::CEngineContext::ActivateOGLContext()
{
	m_parent.m_context.setActive(true);
}

void InjectedEngineRunner::CEngineContext::DeactivateOGLContext()
{
	m_parent.m_context.setActive(false);
}

void InjectedEngineRunner::ThreadProc()
{
	util::dcout << "Engine runner thread entry @ " << GetCurrentThreadId() << std::endl;
	ScopedOGLContext<sf::Context> sgolc(m_context);

	GetFrontRenderTarget().MakeCurrent();

	MainLoop();

	m_rt1.DeInit();
	m_rt2.DeInit();
}

void InjectedEngineRunner::FlipBuffers()
{
	glFlush();
	
	bool currentFlip = m_flipTargets.load();
	// do atomic compare-exchange, should not fail
	// because only one thread, this one, writes
	// to the variable.
	if(!m_flipTargets.compare_exchange_weak(currentFlip, !currentFlip))
	{
		DebugBreak();
	}

	m_rendererThread.QueueUpdate();
	GetFrontRenderTarget().MakeCurrent();
}

InjectedEngineRunner::RenderTarget &InjectedEngineRunner::GetFrontRenderTarget()
{
	return m_flipTargets ? m_rt2 : m_rt1;
}

InjectedEngineRunner::RenderTarget &InjectedEngineRunner::GetBackRenderTarget()
{
	return m_flipTargets ? m_rt1 : m_rt2;
}

