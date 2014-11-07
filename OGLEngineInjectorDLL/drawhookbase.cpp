#include "drawhookbase.h"


void DrawHookBase::InitDrawHook(HWND window, bool useAlpha)
{
	RECT clientRect;
	GetClientRect(window, &clientRect);
	m_windowWidth = clientRect.right - clientRect.left;
	m_windowHeight = clientRect.bottom - clientRect.top;

	m_memoryDC.reset(new MemoryDC(m_windowWidth, m_windowHeight, useAlpha));
	m_oglContext.reset(new sf::Context());
}

void DrawHookBase::DeInitDrawHook()
{
	m_memoryDC.reset(NULL);
	m_oglContext.reset(NULL);
}
