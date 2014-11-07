#include "textrenderer.h"

#include <Windows.h>
#include <GL/GL.h>


TextRenderer::TextRenderer()
	: m_font(NULL), m_size(30)
{
}

void TextRenderer::SetText(const std::string &str)
{
	m_string = str;

	UpdateTexture();
}

void TextRenderer::SetSize(int size)
{
	m_size = size;

	UpdateTexture();
}

void TextRenderer::SetFont(const sf::Font &font)
{
	m_font = &font;

	UpdateTexture();
}

void TextRenderer::Draw()
{
	if(m_font == NULL)
		return;

	if(m_string.empty())
		return;

	glPushMatrix();
	glScalef(1.0f, -1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	sf::Texture::bind(&m_font->getTexture(m_size), sf::Texture::CoordinateType::Pixels);
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for(auto it = m_string.begin(); it != m_string.end(); it++)
	{
		char c = *it;
		const sf::Glyph &glyph = m_font->getGlyph(c, m_size, false);
		glPushMatrix();
		glTranslatef(glyph.bounds.left, glyph.bounds.top, 0.0f);
		glScalef(glyph.bounds.width, glyph.bounds.height, 0.0f);

		float coordLeft = glyph.textureRect.left;
		float coordTop = glyph.textureRect.top;
		float coordRight = glyph.textureRect.left + glyph.textureRect.width;
		float coordBottom = glyph.textureRect.top + glyph.textureRect.height;

		glBegin(GL_QUADS);
			glTexCoord2f(coordLeft, coordTop);
			glVertex2f(0.0f, 0.0f);

			glTexCoord2f(coordLeft, coordBottom);
			glVertex2f(0.0f, 1.0f);

			glTexCoord2f(coordRight, coordBottom);
			glVertex2f(1.0f, 1.0f);

			glTexCoord2f(coordRight, coordTop);
			glVertex2f(1.0f, 0.0f);
		glEnd();
		glPopMatrix();
		glTranslatef(glyph.advance, 0.0f, 0.0f);
	}

	glDisable(GL_BLEND);
	sf::Texture::bind(NULL);
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void TextRenderer::UpdateTexture()
{
	if(m_font == NULL)
		return;

	if(m_string.empty())
		return;

	for(auto it = m_string.begin(); it != m_string.end(); it++)
	{
		m_font->getGlyph(*it, m_size, false);
	}
}

