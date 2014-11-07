#ifndef H_TEXTRENDERER
#define H_TEXTRENDERER

#include <string>
#include <SFML/Graphics/Font.hpp>


class TextRenderer
{
public:
	TextRenderer();
	void SetText(const std::string &str);
	void SetSize(int size);
	void SetFont(const sf::Font &font);
	void Draw();
private:
	void UpdateTexture();

	std::string m_string;
	int m_size;
	const sf::Font *m_font;
};


#endif
