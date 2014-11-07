#include "iconshook.h"
#include <utils_win32.h>

#include "oglutils.h"
#include "scopedoglcontext.h"


IconsHook::IconsHook(RendererThread &rendererThread)
	: m_rendererThread(rendererThread)
{
}

IconsHook::~IconsHook()
{
}

bool IconsHook::Hook()
{
	HWND worker;
	HWND defView;
	HWND listView;

	if(!util::FindDesktopWindows(worker, defView, listView))
	{
		return false;
	}

	HookWindow(listView);
	InitDrawHook(listView, true);

	{
		ScopedOGLContext<sf::Context> soglc(*m_oglContext);

		m_texture = oglutil::CreateRGB8Texture(m_windowWidth, m_windowHeight, true);
	}

	m_rendererThread.SetIconsTexture(m_texture);

	return true;
}

bool IconsHook::Unhook()
{
	WindowHooker::UnhookWindow();

	{
		ScopedOGLContext<sf::Context> soglc(*m_oglContext);

		glDeleteTextures(1, &m_texture);
	}
	DeInitDrawHook();

	return true;
}

LRESULT IconsHook::HookWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_PAINT:
		{
			RECT updateRect;
			GetUpdateRect(m_hookedWindow, &updateRect, FALSE);

			HRGN updateRegion = CreateRectRgnIndirect(&updateRect);
			SelectClipRgn(m_memoryDC->GetDC(), updateRegion);
			DeleteObject(updateRegion);
			SendMessage(m_hookedWindow, WM_PRINTCLIENT, (WPARAM)m_memoryDC->GetDC(), PRF_CLIENT);
			GdiFlush();

			m_oglContext->setActive(true);
			glBindTexture(GL_TEXTURE_2D, m_texture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_windowWidth, m_windowHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_memoryDC->GetPixels());
			m_oglContext->setActive(false);

			m_rendererThread.QueueUpdate();

			ValidateRect(m_hookedWindow, &updateRect);

			return 0;
		}
	}

	switch (msg)
	{
		case WM_CLOSE:
		case WM_SIZE:
		case WM_EXITSIZEMOVE:
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
		case WM_CHAR:
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
		case WM_MOUSEWHEEL:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MOUSELEAVE:
		case WM_MOUSEMOVE:
		{
			LRESULT ret = WindowHooker::HookWndProc(msg, wParam, lParam);
			m_rendererThread.EmptyEventQueue();

			return ret;
		}
	}

	return WindowHooker::HookWndProc(msg, wParam, lParam);
}

