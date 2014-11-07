#include "injectorcrtstub.h"

#include <Windows.h>
#include <Psapi.h>
#include <process.h>


#define THREADDATA_SIZE(size) (sizeof(ThreadData) - sizeof(char) + (size))
struct ThreadData
{
	HANDLE processHeap;

	DWORD hookThreadId;
	DWORD wrapperThreadId;
	DWORD crtThreadId;

	HMODULE thisDLL;
	unsigned int dataSize;
	char data[1];
};

// The stub creates two threads, one that executes the injector procedure
// with the CRT and another one not using the CRT to keep the injector
// module loaded. It allows the CRT thread to exit normally and allows the
// CRT to cleanup and be initialised correctly.

unsigned int __stdcall CRTThreadProc(void *threadData)
{
	ThreadData *td = (ThreadData *)threadData;
	td->crtThreadId = GetCurrentThreadId();

	InjectorProc(td->thisDLL, td->dataSize, td->data);

	return 0;
}

DWORD WINAPI WrapperThreadProc(void *threadData)
{
	ThreadData *td = (ThreadData *)threadData;
	td->wrapperThreadId = GetCurrentThreadId();

	HANDLE thread = (HANDLE)_beginthreadex(NULL, 0, CRTThreadProc, td, 0, NULL);
	WaitForSingleObject(thread, INFINITE);
	DWORD retCode;
	GetExitCodeThread(thread, &retCode);
	CloseHandle(thread);

	HMODULE thisDLL = td->thisDLL;
	HANDLE processHeap = td->processHeap;
	HeapFree(processHeap, 0, td);
	FreeLibraryAndExitThread(thisDLL, 42);
}

void INJECTOR_CONVENTION InjectorCrtStubMain(void *injectorModule, unsigned int dataSize, const void *data)
{
	HANDLE processHeap = GetProcessHeap();
	ThreadData *td = (ThreadData *)HeapAlloc(processHeap, 0, THREADDATA_SIZE(dataSize));

	//td->thisDLL = LoadLibrary(InjectorCrtStubDllName);
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)injectorModule, &td->thisDLL);

	td->processHeap = processHeap;
	td->hookThreadId = GetCurrentThreadId();

	td->dataSize = dataSize;
	if(dataSize != 0)
	{
		for (size_t i = 0; i < dataSize; i++)
		{
			td->data[i] = ((const char *)data)[i];
		}
	}

	HANDLE thread = CreateThread(NULL, 0, WrapperThreadProc, td, 0, NULL);
	CloseHandle(thread);
}
