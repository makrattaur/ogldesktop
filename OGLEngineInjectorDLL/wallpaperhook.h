#ifndef H_WALLPAPERHOOK
#define H_WALLPAPERHOOK

#include "inc_ogl.h"
#include <memory>

#include "windowhooker.h"
#include "drawhookbase.h"
#include "rendererthread.h"


class WallpaperHook : public WindowHooker, public DrawHookBase
{
public:
	WallpaperHook(RendererThread &rendererThread);
	virtual ~WallpaperHook();
	bool Hook();
	virtual bool Unhook();
private:
	virtual LRESULT HookWndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void CreateTexture(GLuint &targetTexture);
	void DumpWindow();
	void UpdateTexture(GLuint targetTexture);

	RendererThread &m_rendererThread;
};


#endif

