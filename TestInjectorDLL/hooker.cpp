#include <Windows.h>

#include <iostream>
#include <string>
#include <sstream>

//#define INJECTORCRTSTUB_DLLNAME "TestInjectorDLL.dll"
#include <injectorcrtstub.h>
#include <utils_win32.h>


WNDPROC g_originalWndProc;

LRESULT CALLBACK HookWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_PAINT:
		{
			RECT currentInvalidRect;
			GetUpdateRect(hWnd, &currentInvalidRect, FALSE);
			CallWindowProc(g_originalWndProc, hWnd, msg, wParam, lParam);
			InvalidateRect(hWnd, &currentInvalidRect, FALSE);

			PAINTSTRUCT ps;
			HDC dc = BeginPaint(hWnd, &ps);

			RECT clientRect;
			GetClientRect(hWnd, &clientRect);

			HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
			HPEN oldPen = (HPEN)SelectObject(dc, pen);

			MoveToEx(dc, clientRect.left, clientRect.top, NULL);
			LineTo(dc, clientRect.right, clientRect.bottom);

			MoveToEx(dc, clientRect.right, clientRect.top, NULL);
			LineTo(dc, clientRect.left, clientRect.bottom);

			SelectObject(dc, oldPen);
			DeleteObject(pen);
			EndPaint(hWnd, &ps);

			return 0;
		}
	}

	return CallWindowProc(g_originalWndProc, hWnd, msg, wParam, lParam);
}

HWND GetTargetWindow()
{
#if 1
	HWND desktop = GetDesktopWindow();
	HWND testWindow = FindWindowEx(desktop, NULL, "InjectionTestWindow", "Injection Test Window");

	return testWindow;
#endif
#if 0
	HWND desktop = GetDesktopWindow();
	HWND testWindow = FindWindowEx(desktop, NULL, "MozillaWindowClass", NULL);

	return testWindow;
#endif
}

void InjectorProc(void *injectorModule, unsigned int dataSize, const void *data)
{
	util::dcout << "Hi from HookProc !" << std::endl;

	HWND testWindow = GetTargetWindow();

	if(testWindow == NULL)
	{
		util::dcout << "Cannot find window !" << std::endl;

		return;
	}

	WNDPROC originalWndProc = (WNDPROC)GetWindowLongPtr(testWindow, GWLP_WNDPROC);
	g_originalWndProc = originalWndProc;

	SetWindowLongPtr(testWindow, GWLP_WNDPROC, (LONG_PTR)HookWndProc);
	InvalidateRect(testWindow, NULL, TRUE);

	Sleep(10000);

	SetWindowLongPtr(testWindow, GWLP_WNDPROC, (LONG_PTR)originalWndProc);
	InvalidateRect(testWindow, NULL, TRUE);

	util::dcout << "Exiting HookProc." << std::endl;
}

