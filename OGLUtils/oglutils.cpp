#include "oglutils.h"
#include <cstddef>


void oglutil::DrawTexturedQuad()
{
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.0f, 0.0f);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.0f, 1.0f);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(1.0f, 1.0f);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(1.0f, 0.0f);
	glEnd();
}

void oglutil::DrawColorfulQuad()
{
	glBegin(GL_QUADS);
		glColor3ub(255, 0, 0);
		glVertex2f(0.0f, 0.0f);

		glColor3ub(0, 255, 0);
		glVertex2f(0.0f, 1.0f);

		glColor3ub(0, 0, 255);
		glVertex2f(1.0f, 1.0f);

		glColor3ub(255, 255, 255);
		glVertex2f(1.0f, 0.0f);
	glEnd();
}

void oglutil::DrawColorfulCube()
{
	glBegin(GL_QUADS);
		// Top face (+y)
		glNormal3f(0.0f, 1.0f, 0.0f); // Up normal
		glColor3ub(255, 0, 0);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(0.5f, 0.5f, 0.5f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.5f, 0.5f, -0.5f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-0.5f, 0.5f, -0.5f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-0.5f, 0.5f, 0.5f);

		// Bottom face (-y)
		glNormal3f(0.0f, -1.0f, 0.0f); // Bottom normal
		glColor3ub(0, 255, 0);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(0.5f, -0.5f, 0.5f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.5f, -0.5f, -0.5f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-0.5f, -0.5f, -0.5f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-0.5f, -0.5f, 0.5f);

		// Front face (-z)
		glNormal3f(0.0f, 0.0f, -1.0f); // Front normal
		glColor3ub(0, 0, 255);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(0.5f, 0.5f, -0.5f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.5f, -0.5f, -0.5f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-0.5f, -0.5f, -0.5f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-0.5f, 0.5f, -0.5f);

		// Back face (+z)
		glNormal3f(0.0f, 0.0f, 1.0f); // Back normal
		glColor3ub(255, 0, 255);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(0.5f, 0.5f, 0.5f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.5f, -0.5f, 0.5f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-0.5f, -0.5f, 0.5f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-0.5f, 0.5f, 0.5f);

		// Right face (+x)
		glNormal3f(1.0f, 0.0f, 0.0f); // Right normal
		glColor3ub(255, 255, 0);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(0.5f, -0.5f, 0.5f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.5f, -0.5f, -0.5f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(0.5f, 0.5f, -0.5f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(0.5f, 0.5f, 0.5f);

		// Left face (-x)
		glNormal3f(-1.0f, 0.0f, 0.0f); // Left normal
		glColor3ub(0, 255, 255);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-0.5f, -0.5f, 0.5f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-0.5f, -0.5f, -0.5f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-0.5f, 0.5f, -0.5f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-0.5f, 0.5f, 0.5f);
	glEnd();
}

GLuint oglutil::CreateRGB8Texture(int width, int height, bool useAlpha)
{
	GLuint texture;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		useAlpha ? GL_RGBA8 : GL_RGB8,
		width,
		height,
		0,
		useAlpha ? GL_RGBA : GL_RGB,
		GL_UNSIGNED_BYTE,
		NULL
	);

	return texture;
}

void oglutil::InitOpenGLBase(bool transparentClearColor)
{
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glClearColor(0.0f, 0.0f, 0.0f, !transparentClearColor ? 1.0f : 0.0f);
	glClearDepth(1.0f);
}

void oglutil::InitOpenGLOrthographic(int width, int height, bool transparentClearColor)
{
	InitOpenGLBase(transparentClearColor);

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);
}

