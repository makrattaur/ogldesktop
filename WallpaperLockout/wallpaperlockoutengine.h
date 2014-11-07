#ifndef H_VIDEOPLAYERENGINE
#define H_VIDEOPLAYERENGINE

#include <engine.h>
#define NOMINMAX
#include <Windows.h>
#include <GL/GL.h>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>


class WallpaperLockoutEngine : public Engine
{
public:
	WallpaperLockoutEngine();
	virtual bool Init();
	virtual void Update(float deltaTime);
	virtual void Draw();
	virtual void DeInit();
private:
	void InitOpenGL();
	void DrawTexturedQuad();
	void CalcRectSize(bool fit, float w1, float h1, float w2, float h2, float &fw, float &fh);

	sf::Texture m_texture;
	float m_drawWidth;
	float m_drawHeight;
};


#endif

