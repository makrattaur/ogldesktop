#include <enginedll.h>
#include "testengine.h"


Engine *CreateEngine()
{
	return new TestEngine();
}
