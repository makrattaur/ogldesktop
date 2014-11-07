#include "testengine.h"

#include <iostream>
#include <cmath>

#include <Windows.h>
//#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/GLU.h>

#define ENGINE_ROOT_PATH "C:\\Users\\Michael\\Documents\\Visual Studio 2012\\Projects\\OGLDesktop\\TestOGLEngine\\"
//#define ENGINE_ROOT_PATH "C:\\Users\\Michael\\Desktop\\Release-ds\\"


const float TestEngine::m_cubeRotationAmountX = 20.0f;
const float TestEngine::m_cubeRotationAmountY = 50.0f;

TestEngine::TestEngine()
	: m_cubeRotationX(0.0f), m_cubeRotationY(0.0f)
{
}

bool TestEngine::Init()
{
	//std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	//std::cout << "OpenGL renderer: " << glGetString(GL_RENDERER) << std::endl;
	//std::cout << "OpenGL vendor: " << glGetString(GL_VENDOR) << std::endl;
	//std::cout << "OpenGL extensions: " << glGetString(GL_EXTENSIONS) << std::endl;
	
	int width = GetWidth();
	int height = GetHeight();

	// setup projection matrix
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(0, width, 0, height, -1, 1);
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	sf::Image textureImage;
	textureImage.loadFromFile(ENGINE_ROOT_PATH "test-tile.png");
	textureImage.flipVertically();
	m_texture.loadFromImage(textureImage);

#if 0
	sf::Image subOverlay;
	subOverlay.loadFromFile("..\\TestOGLEngine\\subimg.png");
	//subOverlay.flipVertically();

	sf::Vector2u size = subOverlay.getSize();
	int updateX = 32;
	int updateY = 64;
	int updateW = 96;
	int updateH = 150;

	sf::Texture::bind(&m_texture);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, size.x);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, updateY);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, updateX);
	glTexSubImage2D(GL_TEXTURE_2D, 0, updateX, updateY, updateW, updateH, GL_RGBA, GL_UNSIGNED_BYTE, subOverlay.getPixelsPtr());
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
#endif

	return true;
}

void TestEngine::Update(float deltaTime)
{
	m_cubeRotationX += m_cubeRotationAmountX * deltaTime;
	m_cubeRotationX = fmod(m_cubeRotationX, 360.0f);

	m_cubeRotationY += m_cubeRotationAmountY * deltaTime;
	m_cubeRotationY = fmod(m_cubeRotationY, 360.0f);

	//std::cout << "\rDelta: " << deltaTime << " (" << (1.0f / deltaTime) << " fps)";
}

void TestEngine::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

#if 1
	glTranslatef(0.0f, 0.0f, -3.0f);
	glRotatef(m_cubeRotationY, 0.0f, 1.0f, 0.0f);
	glRotatef(m_cubeRotationX, 1.0f, 0.0f, 0.0f);
	DrawCube();
#endif

	DoDraw2d();
}

int TestEngine::PreferredWidth()
{
	return 1280;
}

int TestEngine::PreferredHeight()
{
	return 720;
}

const char *TestEngine::PreferredTitle()
{
	return "Test OpenGL Window";
}

void TestEngine::DrawQuad()
{
	int width = GetWidth();
	int height = GetHeight();
	glTranslatef(0.0f, 0.0f, 0.0f);
	glScalef(width, height, 0.0f);

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

void TestEngine::DrawTexturedQuad()
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

void TestEngine::DrawTransparentQuad()
{
	//glColor4ub(0, 128, 192, 96);
	float alpha = 96.0f / 255.0f;
	glColor4f(0.0f / 255.0f * alpha, 128.0f / 255.0f * alpha, 192.0f / 255.0f * alpha, alpha);

	glBegin(GL_QUADS);
		//glColor3ub(255, 0, 0);
		glVertex2f(0.0f, 0.0f);

		//glColor3ub(0, 255, 0);
		glVertex2f(0.0f, 1.0f);

		//glColor3ub(0, 0, 255);
		glVertex2f(1.0f, 1.0f);

		//glColor3ub(255, 255, 255);
		glVertex2f(1.0f, 0.0f);
	glEnd();
}

void TestEngine::DrawCube()
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

void TestEngine::DoDraw2d()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, GetWidth(), 0, GetHeight(), -1, 1);

	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	Draw2d();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void TestEngine::Draw2d()
{
#if 1
	glEnable(GL_TEXTURE_2D);
	glColor3ub(255, 255, 255);
	sf::Texture::bind(&m_texture);
	sf::Vector2u size = m_texture.getSize();
	glPushMatrix();
	//glTranslatef(GetWidth() / 2.0f, GetHeight() / 2.0f, 0.0f);
	glScalef(size.x, size.y, 0.0f);
	DrawTexturedQuad();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);

	int width = 1024;
	int height = 512;
	//float width = GetWidth() - 256.0f;
	//float height = GetHeight() - 208.0f;
	//int width = 256;
	//int height = 256;
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glPushMatrix();
	//glTranslatef(0.0f, GetHeight() - 256.0f, 0.0f);
	glTranslatef(GetWidth() / 2.0f - width / 2.0f, GetHeight() / 2.0f - height / 2.0f, 0.0f);
	glScalef(width, height, 0.0f);
	DrawTransparentQuad();
	glPopMatrix();
	glDisable(GL_BLEND);
#endif
#if 0
	glPushMatrix();
	//glScalef(1920.0f, 1080.0f, 0.0f);
	glScalef(1920.0f, 1080.0f, 0.0f);
	DrawPlaceholderQuad();
#endif
}

void TestEngine::DrawPlaceholderQuad()
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
	glPopMatrix();
}

