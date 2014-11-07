#include "engine.h"
#include <cstddef>
#include "ienginecontext.h"


Engine::Engine() :
	m_width(0),
	m_height(0),
	m_quit(false),
	m_hasEngineContext(false),
	m_engineContext(NULL)
{
}

Engine::~Engine()
{
}

bool Engine::Init()
{
	return true;
}

void Engine::Update(float deltaTime)
{
}

void Engine::Draw()
{
}

void Engine::DeInit()
{
}

int Engine::GetWidth()
{
	return m_width;
}

int Engine::GetHeight()
{
	return m_height;
}

bool Engine::GetQuitState()
{
	return m_quit;
}

void Engine::SignalKeyDown(KeyboardKey key)
{
	OnKeyDown(key);
}

void Engine::SignalKeyUp(KeyboardKey key)
{
	OnKeyUp(key);
}

void Engine::SignalMouseButtonDown(MouseButton button, int x, int y)
{
	OnMouseButtonDown(button, x, y);
}

void Engine::SignalMouseButtonUp(MouseButton button, int x, int y)
{
	OnMouseButtonUp(button, x, y);
}

void Engine::SignalMouseWheelMove(int delta, int x, int y)
{
	OnMouseWheelMove(delta, x, y);
}

void Engine::SignalMouseMove(int x, int y)
{
	OnMouseMove(x, y);
}

void Engine::SignalResize(int newW, int newH)
{
	OnResize(newW, newH);

	m_width = newW;
	m_height = newH;
}

void Engine::SignalClose()
{
	OnClose();
}

void Engine::SetEngineContext(IEngineContext &context)
{
	m_engineContext = &context;
	m_width = m_engineContext->GetWidth();
	m_height = m_engineContext->GetHeight();
	m_hasEngineContext = true;
}

int Engine::PreferredWidth()
{
	return 800;
}

int Engine::PreferredHeight()
{
	return 600;
}

const char *Engine::PreferredTitle()
{
	return "OpenGL Engine";
}

void Engine::QuitEngine()
{
	m_quit = true;
}

void Engine::OnKeyDown(KeyboardKey key)
{
}

void Engine::OnKeyUp(KeyboardKey key)
{
}

void Engine::OnMouseButtonDown(MouseButton button, int x, int y)
{
}

void Engine::OnMouseButtonUp(MouseButton button, int x, int y)
{
}

void Engine::OnMouseWheelMove(int delta, int x, int y)
{
}

void Engine::OnMouseMove(int x, int y)
{
}

void Engine::OnResize(int newW, int newH)
{
}

void Engine::OnClose()
{
	QuitEngine();
}

void Engine::ActivateOGLContext()
{
	m_engineContext->ActivateOGLContext();
}

void Engine::DeactivateOGLContext()
{
	m_engineContext->DeactivateOGLContext();
}

IEngineContext *Engine::GetEngineContext()
{
	return m_engineContext;
}

