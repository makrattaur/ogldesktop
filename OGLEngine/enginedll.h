#ifndef G_H_ENGINECREATOR
#define G_H_ENGINECREATOR


class Engine;

extern "C"
{
__declspec(dllexport) Engine *CreateEngine();
}


#endif

