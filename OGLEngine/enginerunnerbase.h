#ifndef G_H_ENGINERUNNERBASE
#define G_H_ENGINERUNNERBASE

#include <memory>
#include "enginecommon.h"

class Engine;
class IEngineContext;


class ENGINE_API EngineRunnerBase
{
public:
	EngineRunnerBase();
	virtual ~EngineRunnerBase();

	void SetEngineContext(IEngineContext &context);
	IEngineContext *GetEngineContext();
	void SetEngine(Engine *engine);
	Engine *GetEngine();

	void SetTargetFrameRate(float targetFrameRate);
	float GetTargetFrameRate();
protected:
	void MainLoop();

	virtual void ProcessEvents(bool &needsToQuit) = 0;
	virtual void SwapBuffers() = 0;
	virtual void MainLoopInit();
	virtual void MainLoopDeInit();
private:
	IEngineContext *m_engineContext;
	std::unique_ptr<Engine> m_engine;

	float m_targetFrameRate;
	float m_iTargetFrameRate;
};


#endif

