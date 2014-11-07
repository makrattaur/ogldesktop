#ifndef G_H_AUTOHANDLETYPES
#define G_H_AUTOHANDLETYPES

#include "autohandle.h"

typedef AutoHandle<HMODULE, FreeLibrary, NULL> ModuleAutoHandle;

#endif
