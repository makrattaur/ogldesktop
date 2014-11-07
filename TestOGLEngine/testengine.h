#ifndef H_TESTENGINE
#define H_TESTENGINE

#include <engine.h>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>


class TestEngine : public Engine
{
public:
	TestEngine();
//	using Engine::Engine;
	virtual bool Init();
	virtual void Draw();
	virtual void Update(float deltaTime);

	virtual int PreferredWidth();
	virtual int PreferredHeight();
	virtual const char *PreferredTitle();
private:
	void DrawQuad();
	void DrawCube();
	void DrawTexturedQuad();
	void DrawTransparentQuad();
	void DoDraw2d();
	void Draw2d();
	void DrawPlaceholderQuad();

	static const float m_cubeRotationAmountX;
	static const float m_cubeRotationAmountY;
	float m_cubeRotationX;
	float m_cubeRotationY;

	sf::Texture m_texture;
};

#endif

