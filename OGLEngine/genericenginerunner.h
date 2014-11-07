#ifndef G_H_ENGINERUNNER
#define G_H_ENGINERUNNER

#include <string>
#include <memory>
#include <SFML/Window.hpp>

#include "enginecommon.h"
#include "context.h"
#include "ienginecontext.h"
#include "enginerunnerbase.h"

class Engine;


class ENGINE_API GenericEngineRunner : public EngineRunnerBase
{
public:
	GenericEngineRunner(Engine *engine);
	virtual ~GenericEngineRunner();
	void Run(const VideoMode &mode, const std::string &title, const ContextSettings &contextSettings);
protected:
	virtual void ProcessEvents(bool &windowQuit);
	virtual void SwapBuffers();
private:
	class CEngineContext : public IEngineContext
	{
	public:
		CEngineContext(GenericEngineRunner &parent);
		virtual int GetWidth();
		virtual int GetHeight();
		virtual void ActivateOGLContext();
		virtual void DeactivateOGLContext();
	private:
		GenericEngineRunner &m_parent;
	};

	static const char *SFMLEventTypeEnumToString(sf::Event::EventType etype);
	static const char *SFMLKeyEnumToString(sf::Keyboard::Key key);
	static const char *SFMLMouseButtonEnumToString(sf::Mouse::Button button);

	sf::Window m_window;

	CEngineContext m_engineContext;
};

#endif

