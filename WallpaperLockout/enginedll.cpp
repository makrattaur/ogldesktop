#include <enginedll.h>
#include "wallpaperlockoutengine.h"

Engine *CreateEngine()
{
	return new WallpaperLockoutEngine();
}
