#include "enginerunnerbase.h"
#include "engine.h"
#include "ienginecontext.h"
#include <SFML\System\Clock.hpp>
#include <SFML\System.hpp>


EngineRunnerBase::EngineRunnerBase()
	: m_engineContext(NULL)
{
	SetTargetFrameRate(60.0f);
}

EngineRunnerBase::~EngineRunnerBase()
{
}

void EngineRunnerBase::SetEngineContext(IEngineContext &context)
{
	m_engineContext = &context;
}

IEngineContext *EngineRunnerBase::GetEngineContext()
{
	return m_engineContext;
}

void EngineRunnerBase::SetEngine(Engine *engine)
{
	m_engine.reset(engine);
}

Engine *EngineRunnerBase::GetEngine()
{
	return m_engine.get();
}

void EngineRunnerBase::SetTargetFrameRate(float targetFrameRate)
{
	m_targetFrameRate = targetFrameRate;
	m_iTargetFrameRate = 1.0f / m_targetFrameRate;
}

float EngineRunnerBase::GetTargetFrameRate()
{
	return m_targetFrameRate;
}

void EngineRunnerBase::MainLoop()
{
	MainLoopInit();

	m_engine->SetEngineContext(*m_engineContext);
	m_engine->Init();

	bool engineQuit = m_engine->GetQuitState();
	bool needsToQuit = false;

	sf::Clock timer;
	float lastProcessingTime = 0.0f;
	float lastWaitTime = 0.0f;

	while (!(engineQuit || needsToQuit))
	{
		timer.restart();

		ProcessEvents(needsToQuit);
		m_engine->Update(lastProcessingTime + lastWaitTime);
		m_engine->Draw();
		SwapBuffers();
		engineQuit = m_engine->GetQuitState();

		lastProcessingTime = timer.getElapsedTime().asSeconds();

		timer.restart();

		float diff = m_iTargetFrameRate - lastProcessingTime;
		if(diff > 0.0f)
		{
			sf::sleep(sf::seconds(diff));
		}

		lastWaitTime = timer.getElapsedTime().asSeconds();
	}

	m_engine->DeInit();

	MainLoopDeInit();
}

void EngineRunnerBase::MainLoopInit()
{
}

void EngineRunnerBase::MainLoopDeInit()
{
}

