#include <SFML\System\Err.hpp>
#include <iomanip>
#include <autohandle.h>
#include <autohandle_win32.h>

#include <enginehost.h>
#include "rendererthread.h"
#include "iconshook.h"
#include "wallpaperhook.h"
#include "injectedenginerunner.h"

//#define INJECTORCRTSTUB_DLLNAME "OGLEngineInjectorDLL.dll"
#include <injectorcrtstub.h>
#include <utils_win32.h>
#include <utils_path.h>

#include <SFML\Window\Context.hpp>


void CheckWindow(HWND window)
{
	bool isUnicode = IsWindowUnicode(window) == TRUE;
	util::dcout << "Is window Unicode: " << (isUnicode ? "true" : "false") << std::endl;

	std::vector<char> buffer(64);

	if(GetClassName(window, buffer.data(), buffer.size()) == 0)
		return;

	HINSTANCE windowHInstance = (HINSTANCE)GetWindowLongPtr(window, GWLP_HINSTANCE);

	WNDCLASSEX classInfo;
	GetClassInfoEx(windowHInstance, buffer.data(), &classInfo);

	LONG_PTR windowUserData = GetWindowLongPtr(window, GWLP_USERDATA);

	util::dcout << "Extra bytes: " << classInfo.cbWndExtra << " (" << (classInfo.cbWndExtra / sizeof(LONG_PTR)) << " pointers of " << sizeof(LONG_PTR) << " bytes)" << std::endl;

	std::ios::fmtflags f(util::dcout.flags());
	util::dcout << "Window user data: 0x" << std::setfill('0') << std::setw(sizeof(LONG_PTR) * 2) << std::hex << windowUserData << std::endl;
	
	util::dcout << "Original window proc ANSI: 0x" << std::setw(sizeof(LONG_PTR) * 2) << classInfo.lpfnWndProc << std::endl;

	{
		std::vector<wchar_t> bufferW(64);
		GetClassNameW(window, bufferW.data(), bufferW.size());

		WNDCLASSEXW classInfoW;
		GetClassInfoExW(windowHInstance, bufferW.data(), &classInfoW);
		util::dcout << "Original window proc Unicode: 0x" << std::setw(sizeof(LONG_PTR) * 2) << classInfoW.lpfnWndProc << std::endl;
	}

	util::dcout.flags(f);
}

#if 0
std::string GetPathToDLL()
{
	ModuleAutoHandle thisDLL(LoadLibrary(INJECTORCRTSTUB_DLLNAME));
	std::vector<char> buffer(1024);
	DWORD size = GetModuleFileName(thisDLL, buffer.data(), buffer.size());

	if(buffer[size - 1] == '\0')
		size--;

	std::string pathToExe(buffer.data(), size);

	return pathToExe.substr(0, pathToExe.find_last_of('\\') + 1);
}
#endif

bool GetEngineCreator(HMODULE injectorModule, ENGINE_PFN_CREATE_ENGINE &getter, HMODULE &engineDll)
{
	//std::string dllPath = GetPathToDLL();
	//dllPath.insert(dllPath.size(), "TestOGLEngine.dll");
	//dllPath.insert(dllPath.size(), "VideoPlayerEngine.dll");
	//dllPath.insert(dllPath.size(), "WallpaperLockout.dll");

	std::string injectorModulePath = util::getModuleDirectory(injectorModule);
	std::string enginePath;
	//enginePath = "TestOGLEngine.dll";
	enginePath = "VideoPlayerEngine.dll";
	//enginePath = "WallpaperLockout.dll";
	//enginePath = "MeshRendererEngine.dll";
	std::string dllPath = util::Path::Combine(injectorModulePath, enginePath);

	ModuleAutoHandle engineLib(LoadLibraryEx(dllPath.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH));
	if(engineLib.IsInvalid())
	{
		return false;
	}

	getter = (ENGINE_PFN_CREATE_ENGINE)GetProcAddress(engineLib, "CreateEngine");
	if(getter == NULL)
	{
		return false;
	}

	engineDll = engineLib.Disown();
	return true;
}

void InjectorProc(void *injectorModule, unsigned int dataSize, const void *data)
{
	util::dcout << "Injected thread entry @ " << GetCurrentThreadId() << std::endl;
	util::dcout << "In inject proc." << std::endl;
	sf::err().rdbuf(util::dcout.rdbuf());

#if 0
	util::dcout << "Waiting for debugger ..." << std::endl;

	if(IsDebuggerPresent() == FALSE)
	{
		while(IsDebuggerPresent() == FALSE)
		{
			Sleep(100);
		}

		DebugBreak();
	}

	util::dcout << "Debugger attached." << std::endl;
#endif

	HWND worker;
	HWND defView;
	HWND listView;

	if(!util::FindDesktopWindows(worker, defView, listView))
	{
		return;
	}

	//CheckWindow(listView);

	HWND shellWindow = GetShellWindow();

	RendererThread rendererThread;

	HMODULE tempEngineDll;
	ENGINE_PFN_CREATE_ENGINE creator;
	if(!GetEngineCreator((HMODULE)injectorModule, creator, tempEngineDll))
	{
		return;
	}
	ModuleAutoHandle engineDll(tempEngineDll);

#if 1
	InjectedEngineRunner engineRunner(creator(), rendererThread);
#endif
	IconsHook iconsHook(rendererThread);
	WallpaperHook wallpaperHook(rendererThread);

	rendererThread.Init();
	rendererThread.Start();

	iconsHook.Hook();
	wallpaperHook.Hook();

	InvalidateRect(listView, NULL, TRUE);
	//InvalidateRect(shellWindow, NULL, TRUE);
#if 1
	int index = 1;
	engineRunner.Init(rendererThread.GetMonitorWidth(index),
		rendererThread.GetMonitorHeight(index)
	);
	//engineRunner.Init(1280, 1024);
	engineRunner.Start();
	rendererThread.SetEngine(engineRunner, index);
#endif
	//Sleep(30000);
	//Sleep(2 * 60 * 1000);
	{
		std::string stopEventName((const char *)data, dataSize - 1);

		GenericAutoHandle stopEvent(OpenEvent(SYNCHRONIZE, FALSE, stopEventName.c_str()));
		WaitForSingleObject(stopEvent, INFINITE);
	}

	wallpaperHook.Unhook();
	iconsHook.Unhook();

	rendererThread.Stop();
	rendererThread.Deinit();
#if 1
	engineRunner.Stop();
#endif
	InvalidateRect(listView, NULL, TRUE);
	//InvalidateRect(shellWindow, NULL, TRUE);

	util::dcout << "Exiting inject proc." << std::endl;
}

