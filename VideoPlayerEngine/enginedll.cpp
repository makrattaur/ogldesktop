#include <enginedll.h>
#include "videoplayerengine.h"

Engine *CreateEngine()
{
	return new VideoPlayerEngine();
}
