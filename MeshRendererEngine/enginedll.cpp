#include <enginedll.h>
#include "meshrendererengine.h"


Engine *CreateEngine()
{
	return new MeshRendererEngine();
}

