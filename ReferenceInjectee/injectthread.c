#include <Windows.h>


typedef HMODULE (WINAPI *PFN_LOADLIBRARYA)(LPCSTR lpLibFileName);
typedef FARPROC (WINAPI *PFN_GETPROCADDRESS)(HMODULE hModule, LPCSTR lpProcName);
typedef VOID (WINAPI *PFN_EXITPROCESS)(UINT uExitCode);
typedef VOID (WINAPI *PFN_EXITTHREAD)(DWORD dwExitCode);
typedef VOID (WINAPI *PFN_FREELIBRARYANDEXITTHREAD)(HMODULE hModule, DWORD dwExitCode);
typedef int (WINAPI *PFN_MESSAGEBOXA)(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
typedef void (__stdcall *PFN_INJECTPROC)(void);

DWORD WINAPI InjectThreadProc(LPVOID lpThreadParameter)
{
	PFN_LOADLIBRARYA loadLibraryA = LoadLibraryA;
	PFN_GETPROCADDRESS getProcAddress = GetProcAddress;
	PFN_EXITPROCESS exitProcess = ExitProcess;
	PFN_EXITTHREAD exitThread = ExitThread;
	PFN_FREELIBRARYANDEXITTHREAD freeLibraryAndExitThread = FreeLibraryAndExitThread;

	HMODULE user32 = loadLibraryA("user32.dll");
	PFN_MESSAGEBOXA messageBoxA = (PFN_MESSAGEBOXA)getProcAddress(user32, "MessageBoxA");

	HMODULE injectedLib = loadLibraryA("injected.dll");
	PFN_INJECTPROC injectProc;


	if(injectedLib == NULL)
	{
		messageBoxA(0, "Falied to load the DLL injected.dll", "Error", MB_ICONERROR);
		exitProcess(0);
	}

	//PFN_INJECTPROC injectProc = (PFN_INJECTPROC)getProcAddress(injectedLib, "InjectProc");
	injectProc = (PFN_INJECTPROC)getProcAddress(injectedLib, "InjectProc");
	if(injectProc == NULL)
	{
		messageBoxA(0, "Falied to get the function InjectProc", "Error", MB_ICONERROR);
		exitProcess(0);
	}

	injectProc();

#if 1
	exitThread(0);
#endif
#if 1
	freeLibraryAndExitThread(injectedLib, 0);
#endif

	return 0;
}

PFN_LOADLIBRARYA loadLibraryA = LoadLibraryA;
PFN_GETPROCADDRESS getProcAddress = GetProcAddress;
PFN_EXITPROCESS exitProcess = ExitProcess;
PFN_EXITTHREAD exitThread = ExitThread;
PFN_FREELIBRARYANDEXITTHREAD freeLibraryAndExitThread = FreeLibraryAndExitThread;

DWORD WINAPI InjectThreadProc2(LPVOID lpThreadParameter)
{
	HMODULE user32 = loadLibraryA("user32.dll");
	PFN_MESSAGEBOXA messageBoxA = (PFN_MESSAGEBOXA)getProcAddress(user32, "MessageBoxA");

	HMODULE injectedLib = loadLibraryA("injected.dll");
	PFN_INJECTPROC injectProc;

	if(injectedLib == NULL)
	{
		messageBoxA(0, "Falied to load the DLL injected.dll", "Error", MB_ICONERROR);
		exitProcess(0);
	}

	//PFN_INJECTPROC injectProc = (PFN_INJECTPROC)getProcAddress(injectedLib, "InjectProc");
	injectProc = (PFN_INJECTPROC)getProcAddress(injectedLib, "InjectProc");
	if(injectProc == NULL)
	{
		messageBoxA(0, "Falied to get the function InjectProc", "Error", MB_ICONERROR);
		exitProcess(0);
	}

	injectProc();

#if 1
	exitThread(0);
#endif
#if 1
	freeLibraryAndExitThread(injectedLib, 0);
#endif

	return 0;
}

DWORD WINAPI InjectThreadProc3(LPVOID lpThreadParameter)
{
	return 0;
}

DWORD WINAPI InjectThreadProc4(LPVOID lpThreadParameter)
{
	ExitThread(0);
	return 0;
}

void DoIt()
{
	CreateThread(NULL, 0, InjectThreadProc2, NULL, 0, NULL);
}

int main(int argc, char *argv[])
{
	int j = 0;
	return 0;
}

