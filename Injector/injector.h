#ifndef H_INJECTORX64
#define H_INJECTORX64

#include <Windows.h>

struct InjectConfig
{
	InjectConfig();
	bool silent;
	bool use64Bit;
	bool useDebug;
	const char *dllPath;
	const char *functionName;
	unsigned int dataSize;
	const void *data;
};

bool InjectProcess64(HANDLE process, const char *dllPath, const char *functionName, bool silent = false);
bool InjectProcess32(HANDLE process, const char *dllPath, const char *functionName, bool silent = false);
bool InjectProcess64(HANDLE process, InjectConfig &ic);
bool InjectProcess32(HANDLE process, InjectConfig &ic);

#endif

