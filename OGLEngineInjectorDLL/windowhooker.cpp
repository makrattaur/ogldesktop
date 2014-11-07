#include "windowhooker.h"


const char *WindowHooker::m_PropKeyString = "OGLInjectPropKey";

bool WindowHooker::HookWindow(HWND window)
{
	m_propertyKey = GlobalAddAtom(m_PropKeyString);
	m_hookedWindow = window;

	SetProp(m_hookedWindow, GetPropertyKey(), this);

	m_originalWndProc = (WNDPROC)GetWindowLongPtr(m_hookedWindow, GWLP_WNDPROC);
	SetWindowLongPtr(m_hookedWindow, GWLP_WNDPROC, (LONG_PTR)&WindowHooker::HookWndProcTrampoline);

	return true;
}

bool WindowHooker::UnhookWindow()
{
	SetWindowLongPtr(m_hookedWindow, GWLP_WNDPROC, (LONG_PTR)m_originalWndProc);
	RemoveProp(m_hookedWindow, GetPropertyKey());

	return true;
}

WindowHooker::~WindowHooker()
{
	GlobalDeleteAtom(m_propertyKey);
}

WindowHooker *WindowHooker::ExtractData(HWND window)
{
	ATOM propKey = GlobalAddAtom(m_PropKeyString);
	WindowHooker *wh = (WindowHooker *)GetProp(window, (LPCSTR)MAKELONG(propKey, 0));
	GlobalDeleteAtom(propKey);

	return wh;
}

LPCSTR WindowHooker::GetPropertyKey()
{
	return (LPCSTR)MAKELONG(m_propertyKey, 0);
}

LRESULT WindowHooker::HookWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	return CallOriginalWindowProc(msg, wParam, lParam);
}

LRESULT WindowHooker::OuterHookWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_NCDESTROY:
		{
			RemoveProp(m_hookedWindow, GetPropertyKey());

			// fallthrough.
		}
		default:
		{
			return HookWndProc(msg, wParam, lParam);
		}
	}
}

LRESULT WindowHooker::CallOriginalWindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	return CallWindowProc(m_originalWndProc, m_hookedWindow, msg, wParam, lParam);
}

LRESULT CALLBACK WindowHooker::HookWndProcTrampoline(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WindowHooker *wh = WindowHooker::ExtractData(window);

	if(wh == NULL)
	{
		return 0; // !!
	}

	return wh->HookWndProc(msg, wParam, lParam);
}
