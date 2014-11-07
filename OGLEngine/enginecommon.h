#ifndef G_H_ENGINECOMMON
#define G_H_ENGINECOMMON

#ifndef BUILDING_LIBRARY_STATIC
#ifdef BUILDING_LIBRARY
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif
#else
#define ENGINE_API
#endif


#endif

