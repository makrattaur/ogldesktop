#ifndef G_H_INJECTORCRTSTUB
#define G_H_INJECTORCRTSTUB


#define INJECTOR_CONVENTION __stdcall

extern "C"
{
	__declspec(dllexport) void INJECTOR_CONVENTION InjectorCrtStubMain(void *injectorModule, unsigned int dataSize, const void *data);
}

void InjectorProc(void *injectorModule, unsigned int dataSize, const void *data);
extern const char *InjectorCrtStubDllName;

#ifdef INJECTORCRTSTUB_DLLNAME
const char *InjectorCrtStubDllName = INJECTORCRTSTUB_DLLNAME;
#include "utilmacros.h"
#pragma WARNING("Symbol InjectorCrtStubDllName is no longer used.")
#endif

#endif

