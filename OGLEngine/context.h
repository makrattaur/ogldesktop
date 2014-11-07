#ifndef G_H_CONTEXT
#define G_H_CONTEXT

#include "enginecommon.h"


class ENGINE_API VideoMode
{
public:
	VideoMode(int width, int height, int bitsPerPixel = 32);
	int GetWidth() const;
	int GetHeight() const;
	int GetBitsPerPixel() const;
private:
	int m_width;
	int m_height;
	int m_bitsPerPixel;
};

class ENGINE_API ContextSettings
{
public:
	ContextSettings(int depthBits, int stencilBits);
	int GetDepthBits() const;
	int GetStencilBits() const;
private:
	int m_depthBits;
	int m_stencilBits;
};


#endif

