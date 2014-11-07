#ifndef H_DRAWHOOKBASE
#define H_DRAWHOOKBASE

#include <memory>
#include <Windows.h>
#include <SFML\Window\Context.hpp>

#include "..\TestOGLInjectorDLL\memorydc.h"

class DrawHookBase
{
protected:
	void InitDrawHook(HWND window, bool useAlpha);
	void DeInitDrawHook();

	std::unique_ptr<MemoryDC> m_memoryDC;
	std::unique_ptr<sf::Context> m_oglContext;
	int m_windowWidth;
	int m_windowHeight;
};

#endif
