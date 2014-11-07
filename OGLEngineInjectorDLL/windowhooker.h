#ifndef H_WINDOWHOOKER
#define H_WINDOWHOOKER

#include <Windows.h>


class WindowHooker
{
public:
	virtual bool HookWindow(HWND window);
	virtual bool UnhookWindow();
	virtual ~WindowHooker();
protected:
	virtual LRESULT HookWndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT OuterHookWndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CallOriginalWindowProc(UINT msg, WPARAM wParam, LPARAM lParam);

	HWND m_hookedWindow;
private:
	static WindowHooker *ExtractData(HWND window);
	LPCSTR GetPropertyKey();
	static LRESULT CALLBACK HookWndProcTrampoline(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

	static const char *m_PropKeyString;
	ATOM m_propertyKey;
	WNDPROC m_originalWndProc;
};

#endif
