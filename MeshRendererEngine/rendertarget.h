#ifndef H_RENDERTARGET
#define H_RENDERTARGET

#include "ogl_inc.h"


class RenderTarget
{
public:
	RenderTarget();
	~RenderTarget();

	void Init(int width, int height);
	void DeInit();
	GLuint GetTexture();

	static void MakeCurrent(const RenderTarget *rt);
private:
	bool m_destroyed;
	GLuint m_depthBuffer;
	GLuint m_targetTexture;
	GLuint m_frameBuffer;
};


#endif

