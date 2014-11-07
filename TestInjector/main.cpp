#include <string>
#include <vector>

#include <autohandle_win32.h>
#include <injector.h>
#include <utils_win32.h>

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

int main(int argc, char *argv[])
{
#if 0
	HWND worker;
	HWND defView;
	HWND listView;

	if(!FindDesktopWindows(worker, defView, listView))
	{
		return 1;
	}
#endif

	HWND testWindow = GetTargetWindow();

	DWORD pid;
	DWORD tid = GetWindowThreadProcessId(testWindow, &pid);
	GenericAutoHandle process(OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION, FALSE, pid));

	if(process.IsInvalid())
	{
		return 1;
	}

	std::string dllPath = util::getExeDirectory();
	dllPath.insert(dllPath.size(), "TestInjectorDLL.dll");
	//dllPath.insert(dllPath.size(), "TestOGLInjectorDLL.dll");
	//InjectProcess64(process, dllPath.c_str(), "InjectorCrtStubMain");
	InjectProcess32(process, dllPath.c_str(), "InjectorCrtStubMain");

	return 0;
}

