#ifndef G_H_IENGINECONTEXT
#define G_H_IENGINECONTEXT

#include "enginecommon.h"


class ENGINE_API IEngineContext
{
public:
	virtual ~IEngineContext() { }
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual void ActivateOGLContext() = 0;
	virtual void DeactivateOGLContext() = 0;
};


#endif
