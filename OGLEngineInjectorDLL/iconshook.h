#ifndef H_ICONSHOOK
#define H_ICONSHOOK

#include "inc_ogl.h"

#include "windowhooker.h"
#include "drawhookbase.h"
#include "rendererthread.h"


class IconsHook : public WindowHooker, public DrawHookBase
{
public:
	IconsHook(RendererThread &rendererThread);
	virtual ~IconsHook();
	bool Hook();
	virtual bool Unhook();
private:
	virtual LRESULT HookWndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	RendererThread &m_rendererThread;
	GLuint m_texture;
};


#endif

