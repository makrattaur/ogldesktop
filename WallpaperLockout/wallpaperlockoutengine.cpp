#include "wallpaperlockoutengine.h"
#include "resload.h"


WallpaperLockoutEngine::WallpaperLockoutEngine()
{
}

bool WallpaperLockoutEngine::Init()
{
	InitOpenGL();

	sf::Image textureImage;
	//textureImage.loadFromFile("C:\\Users\\Michael\\wallpapers\\lugarescoloridos8.jpg");
	//textureImage.loadFromFile("C:\\Users\\Michael\\Desktop\\owmfgo2.jpg");
	//textureImage.loadFromFile("C:\\Users\\Michael\\wallpapers\\Earth.jpg");
	//textureImage.loadFromFile("C:\\Users\\Michael\\wallpapers\\glacier1920x1200.jpg");
	LoadImageFromResources(textureImage);
	textureImage.flipVertically();
	m_texture.loadFromImage(textureImage);

	sf::Vector2u textureSize = m_texture.getSize();
	CalcRectSize(false, textureSize.x, textureSize.y, GetWidth(), GetHeight(), m_drawWidth, m_drawHeight);

	return true;
}

void WallpaperLockoutEngine::Update(float deltaTime)
{

}

void WallpaperLockoutEngine::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	sf::Texture::bind(&m_texture);

	glColor3ub(255, 255, 255);
	glPushMatrix();
	glTranslatef(GetWidth() / 2.0f - m_drawWidth / 2.0f, GetHeight() / 2.0f - m_drawHeight / 2.0f, 0.0f);
	glScalef(m_drawWidth, m_drawHeight, 0.0f);
	DrawTexturedQuad();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
}

void WallpaperLockoutEngine::DeInit()
{
}

void WallpaperLockoutEngine::InitOpenGL()
{
	int width = GetWidth();
	int height = GetHeight();

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
}

void WallpaperLockoutEngine::DrawTexturedQuad()
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

void WallpaperLockoutEngine::CalcRectSize(bool fit, float w1, float h1, float w2, float h2, float &fw, float &fh)
{
	float wScale = w2 / w1;
	float hScale = h2 / h1;

	float scale = fit ? std::min(hScale, wScale) : std::max(hScale, wScale);

	fw = w1 * scale;
	fh = h1 * scale;
}
