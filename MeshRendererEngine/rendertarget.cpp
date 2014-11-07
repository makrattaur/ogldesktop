#include "rendertarget.h"
#include <oglutils.h>


RenderTarget::RenderTarget()
	: m_destroyed(true),
	m_depthBuffer(0),
	m_targetTexture(0),
	m_frameBuffer(0)
{
}

RenderTarget::~RenderTarget()
{
	DeInit();
}

void RenderTarget::Init(int width, int height)
{
	if(!m_destroyed)
	{
		DeInit();
	}

	glGenFramebuffersEXT(1, &m_frameBuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frameBuffer);
	
	m_targetTexture = oglutil::CreateRGB8Texture(width, height, true);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_targetTexture, 0);

	glGenRenderbuffersEXT(1, &m_depthBuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthBuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_depthBuffer);

	GLenum ret = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(ret != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		DebugBreak();
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	m_destroyed = false;
}

void RenderTarget::DeInit()
{
	if(m_destroyed)
	{
		return;
	}

	if(m_depthBuffer != 0)
	{
		glDeleteRenderbuffersEXT(1, &m_depthBuffer);
		m_depthBuffer = 0;
	}

	if(m_targetTexture != 0)
	{
		glDeleteTextures(1, &m_targetTexture);
		m_targetTexture = 0;
	}

	if(m_frameBuffer != 0)
	{
		glDeleteFramebuffersEXT(1, &m_frameBuffer);
		m_frameBuffer = 0;
	}

	m_destroyed = true;
}

GLuint RenderTarget::GetTexture()
{
	return m_targetTexture;
}

void RenderTarget::MakeCurrent(const RenderTarget *rt)
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rt != NULL ? rt->m_frameBuffer : 0);
}

