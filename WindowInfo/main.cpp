#include <Windows.h>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>


HWND GetTargetWindow()
{
	HWND desktop = GetDesktopWindow();
	HWND testWindow = FindWindowEx(desktop, NULL, "Notepad++", NULL);
	//HWND testWindow = FindWindowEx(desktop, NULL, "InjectionTestWindow", NULL);

	return testWindow;
}

void DisplayRect(RECT &rect)
{
	LONG width = rect.right - rect.left;
	LONG height = rect.bottom - rect.top;

	std::cout << "(" << rect.left << ", " << rect.top << ") - " <<
		"(" << rect.right << ", " << rect.bottom << ") " <<
		width << "x" << height;
}

void ShowInfoUnicode(HWND window)
{
	std::vector<wchar_t> windowTitle;
	windowTitle.resize(GetWindowTextLengthW(window) + 1);
	GetWindowTextW(window, windowTitle.data(), windowTitle.size());
	windowTitle.pop_back();

	std::cout << "Window title: " << std::string(windowTitle.begin(), windowTitle.end()) << std::endl;

	std::cout << "Window proc: " << GetWindowLongPtrW(window, GWLP_WNDPROC) << std::endl;

	std::cout << "Window rect: ";
	RECT windowRect;
	GetWindowRect(window, &windowRect);
	DisplayRect(windowRect);
	std::cout << std::endl;

	std::cout << "Client rect: ";
	RECT clientRect;
	GetClientRect(window, &clientRect);
	DisplayRect(clientRect);
	std::cout << std::endl;

	HINSTANCE windowHInstance = (HINSTANCE)GetWindowLongPtrW(window, GWLP_HINSTANCE);
	std::cout << std::hex;
	std::cout << "Window HINSTANCE: " << windowHInstance << std::endl;
	std::cout << "Window menu: " << GetMenu(window) << std::endl;
	std::cout << "Window userdata: " << GetWindowLongPtrW(window, GWLP_USERDATA) << std::endl;

	std::cout << "Styles: " << GetWindowLongW(window, GWL_STYLE) << std::endl;
	std::cout << "Extended styles: " << GetWindowLongW(window, GWL_EXSTYLE) << std::endl;
	std::cout << std::dec;

	std::vector<wchar_t> buffer(64);
	GetClassNameW(window, buffer.data(), buffer.size());
	std::string windowClassName(buffer.begin(), std::find(buffer.begin(), buffer.end(), L'\0'));

	std::cout << "Class name: " << windowClassName << std::endl;
	std::cout << std::hex;
	std::cout << "Class styles: " << GetClassLongW(window, GCL_STYLE) << std::endl;
	std::cout << std::dec;
	std::cout << "Class bytes: " << GetClassLongW(window, GCL_CBCLSEXTRA) << std::endl;
	std::cout << std::hex;
	std::cout << "Class atom: " << GetClassLongW(window, GCW_ATOM) << std::endl;
	std::cout << std::dec;
	std::cout << "Class window extra bytes: " << GetClassLongW(window, GCL_CBWNDEXTRA) << std::endl;
	std::cout << std::hex;
	std::cout << "Class icon: " << GetClassLongPtrW(window, GCLP_HICON) << std::endl;
	std::cout << "Class cursor: " << GetClassLongPtrW(window, GCLP_HCURSOR) << std::endl;
	std::cout << "Class bg brush: " << GetClassLongPtrW(window, GCLP_HBRBACKGROUND) << std::endl;
	std::cout << "Class HINSTANCE: " << GetClassLongPtrW(window, GCLP_HMODULE) << std::endl;
	std::cout << "Class WndProc: " << GetClassLongPtrW(window, GCLP_WNDPROC) << std::endl;
	std::cout << std::dec;

	DWORD pid;
	DWORD tid = GetWindowThreadProcessId(window, &pid);
	std::cout << "PID: " << pid << std::endl;
	std::cout << "TID: " << tid << std::endl;

	WINDOWPLACEMENT placementInfo;
	placementInfo.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(window, &placementInfo);

	std::cout << "Restaured rect: ";
	DisplayRect(placementInfo.rcNormalPosition);
	std::cout << std::endl;

	std::cout << std::hex;
	std::cout << "Window placement flags: " << placementInfo.showCmd << std::endl;
	std::cout << std::dec;
}

void ShowInfoANSI(HWND window)
{
	std::vector<char> windowTitle;
	windowTitle.resize(GetWindowTextLengthA(window) + 1);
	GetWindowTextA(window, windowTitle.data(), windowTitle.size());
	windowTitle.pop_back();

	std::cout << "Window title: " << std::string(windowTitle.begin(), windowTitle.end()) << std::endl;

	std::cout << "Window proc: " << GetWindowLongPtrA(window, GWLP_WNDPROC) << std::endl;

	std::cout << "Window rect: ";
	RECT windowRect;
	GetWindowRect(window, &windowRect);
	DisplayRect(windowRect);
	std::cout << std::endl;

	std::cout << "Client rect: ";
	RECT clientRect;
	GetClientRect(window, &clientRect);
	DisplayRect(clientRect);
	std::cout << std::endl;

	HINSTANCE windowHInstance = (HINSTANCE)GetWindowLongPtrA(window, GWLP_HINSTANCE);
	std::cout << std::hex;
	std::cout << "Window HINSTANCE: " << windowHInstance << std::endl;
	std::cout << "Window menu: " << GetMenu(window) << std::endl;
	std::cout << "Window userdata: " << GetWindowLongPtrA(window, GWLP_USERDATA) << std::endl;

	std::cout << "Styles: " << GetWindowLongA(window, GWL_STYLE) << std::endl;
	std::cout << "Extended styles: " << GetWindowLongA(window, GWL_EXSTYLE) << std::endl;
	std::cout << std::dec;

#if 0
	std::vector<char> windowClassName(64);
	if(GetClassNameA(window, windowClassName.data(), windowClassName.size()) == 0)
		return;

	WNDCLASSA classInfo;
	//ZeroMemory(&classInfo, sizeof(WNDCLASSEXA));
	//classInfo.cbSize = sizeof(WNDCLASSEXA);
	ATOM classAtom = (ATOM)GetClassInfoA(windowHInstance, windowClassName.data(), &classInfo);

	HRESULT hr = HRESULT_FROM_WIN32(GetLastError());

	std::cout << "Class name: " << std::string(windowClassName.begin(), windowClassName.end() - 1) << std::endl;
	std::cout << std::hex;
	std::cout << "Class styles: " << classInfo.style << std::endl;
	std::cout << "Class bytes: " << classInfo.cbClsExtra << std::endl;
	std::cout << "Class atom: " << classAtom << std::endl;
	std::cout << "Class window extra bytes: " << classInfo.cbWndExtra << std::endl;
	std::cout << "Class icon: " << classInfo.hIcon << std::endl;
	std::cout << "Class cursor: " << classInfo.hCursor << std::endl;
	std::cout << "Class bg brush: " << classInfo.hbrBackground << std::endl;
	std::cout << "Class HINSTANCE: " << classInfo.hInstance << std::endl;
	std::cout << "Class WndProc: " << classInfo.lpfnWndProc << std::endl;
	std::cout << std::dec;

#endif
	std::vector<char> buffer(64);
	GetClassNameA(window, buffer.data(), buffer.size());
	std::string windowClassName(buffer.begin(), std::find(buffer.begin(), buffer.end(), '\0'));

	std::cout << "Class name: " << windowClassName << std::endl;
	std::cout << std::hex;
	std::cout << "Class styles: " << GetClassLongA(window, GCL_STYLE) << std::endl;
	std::cout << std::dec;
	std::cout << "Class bytes: " << GetClassLongA(window, GCL_CBCLSEXTRA) << std::endl;
	std::cout << std::hex;
	std::cout << "Class atom: " << GetClassLongA(window, GCW_ATOM) << std::endl;
	std::cout << std::dec;
	std::cout << "Class window extra bytes: " << GetClassLongA(window, GCL_CBWNDEXTRA) << std::endl;
	std::cout << std::hex;
	std::cout << "Class icon: " << GetClassLongPtrA(window, GCLP_HICON) << std::endl;
	std::cout << "Class cursor: " << GetClassLongPtrA(window, GCLP_HCURSOR) << std::endl;
	std::cout << "Class bg brush: " << GetClassLongPtrA(window, GCLP_HBRBACKGROUND) << std::endl;
	std::cout << "Class HINSTANCE: " << GetClassLongPtrA(window, GCLP_HMODULE) << std::endl;
	std::cout << "Class WndProc: " << GetClassLongPtrA(window, GCLP_WNDPROC) << std::endl;
	std::cout << std::dec;

	DWORD pid;
	DWORD tid = GetWindowThreadProcessId(window, &pid);
	std::cout << "PID: " << pid << std::endl;
	std::cout << "TID: " << tid << std::endl;

	WINDOWPLACEMENT placementInfo;
	placementInfo.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(window, &placementInfo);

	std::cout << "Restaured rect: ";
	DisplayRect(placementInfo.rcNormalPosition);
	std::cout << std::endl;

	std::cout << std::hex;
	std::cout << "Window placement flags: " << placementInfo.showCmd << std::endl;
	std::cout << std::dec;
}

int main(int argc, char *argv[])
{
	HWND window = GetTargetWindow();
	if(window == NULL)
	{
		return 1;
	}

	//HINSTANCE windowHInstance = (HINSTANCE)GetWindowLongPtr(window, GWLP_HINSTANCE);

	bool isUnicode = IsWindowUnicode(window) == TRUE;
	if(isUnicode)
	{
		ShowInfoUnicode(window);
	}
	else
	{
		ShowInfoANSI(window);
	}

	return 0;
}

