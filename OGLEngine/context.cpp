#include "context.h"


VideoMode::VideoMode(int width, int height, int bitsPerPixel)
	: m_width(width), m_height(height), m_bitsPerPixel(bitsPerPixel)
{
}

int VideoMode::GetWidth() const
{
	return m_width;
}

int VideoMode::GetHeight() const
{
	return m_height;
}

int VideoMode::GetBitsPerPixel() const
{
	return m_bitsPerPixel;
}

ContextSettings::ContextSettings(int depthBits, int stencilBits)
	: m_depthBits(depthBits), m_stencilBits(stencilBits)
{
}

int ContextSettings::GetDepthBits() const
{
	return m_depthBits;
}

int ContextSettings::GetStencilBits() const
{
	return m_stencilBits;
}

