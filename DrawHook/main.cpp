#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>

#include <injector.h>
#include <utils_win32.h>
#include <utils_path.h>
#include <autohandle_win32.h>


void EnsureSmoothWallpaperTransition()
{
	HWND window = FindWindowEx(GetDesktopWindow(), NULL, "Progman", NULL);
	if(window != NULL)
	{
		SendMessageTimeout(window, 0x52c, 0, 0, 0, 500, NULL);
	}
}

std::string g_stopEventName("OGLDesktopEvent");
int main2(int argc, char *argv[])
{
	GenericAutoHandle stopEvent(CreateEvent(NULL, FALSE, FALSE, g_stopEventName.c_str()));
	SetEvent(stopEvent);

	return 0;
}

int main(int argc, char *argv[])
{
	EnsureSmoothWallpaperTransition();

	HWND worker;
	HWND defView;
	HWND listView;

	if(!util::FindDesktopWindows(worker, defView, listView))
	{
		return 1;
	}

	HANDLE process;
	DWORD pid;
	DWORD tid = GetWindowThreadProcessId(listView, &pid);
	process = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION, FALSE, pid);

	if(process == NULL)
	{
		return 1;
	}

	//std::string injectorPath = util::getWorkingDirectory();
	//injectorPath.insert(injectorPath.size(), "\\DrawHookInjectee.dll");
	//InjectProcess64(process, injectorPath.c_str(), "InjectorCrtStubMain");
	//Sleep(5000);

	//std::string dllPath = util::getExeDirectory();
	//dllPath.insert(dllPath.size(), "DrawHookInjectee.dll");
	//dllPath.insert(dllPath.size(), "TestOGLInjectorDLL.dll");
	//dllPath.insert(dllPath.size(), "OGLEngineInjectorDLL.dll");
	std::string dllName = "OGLEngineInjectorDLL.dll";
	std::string dllPath = util::Path::Combine(util::getExeDirectory(), dllName);

	InjectConfig ic;
	ic.dllPath = dllPath.c_str();
	ic.functionName = "InjectorCrtStubMain";
	ic.useDebug = false;

	//unsigned int data = 0xbebafeca;
	//ic.data = &data;
	//ic.dataSize = sizeof(data);

	ic.data = g_stopEventName.c_str();
	ic.dataSize = g_stopEventName.size() + 1;

	GenericAutoHandle stopEvent(CreateEvent(NULL, FALSE, FALSE, g_stopEventName.c_str()));

	InjectProcess64(process, ic);

	std::string dummy;
	std::cin >> dummy;
	SetEvent(stopEvent);

	//InjectProcess64(process, dllPath.c_str(), "InjectorCrtStubMain");

	return 0;
}

