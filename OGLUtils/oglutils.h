#ifndef G_H_OGLUTILS
#define G_H_OGLUTILS

#include <Windows.h>
#include <GL/GL.h>


namespace oglutil
{
	void DrawTexturedQuad();
	void DrawColorfulQuad();
	void DrawColorfulCube();

	GLuint CreateRGB8Texture(int width, int height, bool useAlpha);
	void InitOpenGLBase(bool transparentClearColor = false);
	void InitOpenGLOrthographic(int width, int height, bool transparentClearColor = false);
}


#endif

