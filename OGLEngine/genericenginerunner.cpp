#include "genericenginerunner.h"
#include "engine.h"

#include <iostream>
//#include <GL/glew.h>
#include "sfmlinterop.h"


GenericEngineRunner::GenericEngineRunner(Engine *engine)
	: m_engineContext(*this)
{
	SetEngineContext(m_engineContext);
	SetEngine(engine);
}

GenericEngineRunner::~GenericEngineRunner()
{
}

void GenericEngineRunner::Run(const VideoMode &mode, const std::string &title, const ContextSettings &contextSettings)
{
	m_window.create(sf::VideoMode(mode.GetWidth(), mode.GetHeight(), mode.GetBitsPerPixel()),
		title,
		sf::Style::Close,
		sf::ContextSettings(contextSettings.GetDepthBits(), contextSettings.GetStencilBits())
	);

	m_window.setKeyRepeatEnabled(false);
	m_window.setActive(true);

	//glewExperimental = GL_TRUE;
	//GLenum glewErr = glewInit();
	//bool glewOk = glewErr == GLEW_OK;

	MainLoop();
}

void GenericEngineRunner::ProcessEvents(bool &windowQuit)
{
	sf::Event evt;
	while(m_window.pollEvent(evt))
	{
		//if(evt.type != sf::Event::EventType::MouseMoved)
		//{
		//	std::cout << "Dequeued a event of type " << SFMLEventTypeEnumToString(evt.type) << "." << std::endl;
		//}

		switch(evt.type)
		{
			case sf::Event::EventType::KeyPressed:
			{
				std::cout << "Pressed key " << SFMLKeyEnumToString(evt.key.code) << std::endl;

				GetEngine()->SignalKeyDown(SFMLInterop::ConvertKeyboardKey(evt.key.code));

				break;
			}
			case sf::Event::EventType::KeyReleased:
			{
				std::cout << "Released key " << SFMLKeyEnumToString(evt.key.code) << std::endl;

				GetEngine()->SignalKeyUp(SFMLInterop::ConvertKeyboardKey(evt.key.code));

				break;
			}
			case sf::Event::EventType::MouseButtonPressed:
			{
				std::cout << "Pressed button " << SFMLMouseButtonEnumToString(evt.mouseButton.button) << std::endl;

				GetEngine()->SignalMouseButtonDown(SFMLInterop::ConvertMouseButton(evt.mouseButton.button),
					evt.mouseButton.x,
					evt.mouseButton.y);

				break;
			}
			case sf::Event::EventType::MouseButtonReleased:
			{
				std::cout << "Released button " << SFMLMouseButtonEnumToString(evt.mouseButton.button) << std::endl;

				GetEngine()->SignalMouseButtonUp(SFMLInterop::ConvertMouseButton(evt.mouseButton.button),
					evt.mouseButton.x,
					evt.mouseButton.y);

				break;
			}
			case sf::Event::EventType::MouseWheelMoved:
			{
				GetEngine()->SignalMouseWheelMove(evt.mouseWheel.delta, evt.mouseWheel.x, evt.mouseWheel.y);

				break;
			}
			case sf::Event::EventType::Closed:
			{
				GetEngine()->SignalClose();

				windowQuit = true;
				break;
			}
		}
	}
}

void GenericEngineRunner::SwapBuffers()
{
	m_window.display();
}

GenericEngineRunner::CEngineContext::CEngineContext(GenericEngineRunner &parent)
	: m_parent(parent)
{
}

int GenericEngineRunner::CEngineContext::GetWidth()
{
	return m_parent.m_window.getSize().x;
}

int GenericEngineRunner::CEngineContext::GetHeight()
{
	return m_parent.m_window.getSize().y;
}

void GenericEngineRunner::CEngineContext::ActivateOGLContext()
{
	m_parent.m_window.setActive(true);
}

void GenericEngineRunner::CEngineContext::DeactivateOGLContext()
{
	m_parent.m_window.setActive(false);
}

#define SELF_STR_CASE(x) case x: \
{ \
	return #x; \
}

const char *GenericEngineRunner::SFMLEventTypeEnumToString(sf::Event::EventType etype)
{
	switch (etype)
	{
		SELF_STR_CASE(sf::Event::Closed)
		SELF_STR_CASE(sf::Event::Resized)
		SELF_STR_CASE(sf::Event::LostFocus)
		SELF_STR_CASE(sf::Event::GainedFocus)
		SELF_STR_CASE(sf::Event::TextEntered)
		SELF_STR_CASE(sf::Event::KeyPressed)
		SELF_STR_CASE(sf::Event::KeyReleased)
		SELF_STR_CASE(sf::Event::MouseWheelMoved)
		SELF_STR_CASE(sf::Event::MouseButtonPressed)
		SELF_STR_CASE(sf::Event::MouseButtonReleased)
		SELF_STR_CASE(sf::Event::MouseMoved)
		SELF_STR_CASE(sf::Event::MouseEntered)
		SELF_STR_CASE(sf::Event::MouseLeft)
		SELF_STR_CASE(sf::Event::JoystickButtonPressed)
		SELF_STR_CASE(sf::Event::JoystickButtonReleased)
		SELF_STR_CASE(sf::Event::JoystickMoved)
		SELF_STR_CASE(sf::Event::JoystickConnected)
		SELF_STR_CASE(sf::Event::JoystickDisconnected)
		SELF_STR_CASE(sf::Event::Count)
		default:
		{
			return "<unknown>";
		}
	}
}

const char *GenericEngineRunner::SFMLKeyEnumToString(sf::Keyboard::Key key)
{
	switch (key)
	{
		SELF_STR_CASE(sf::Keyboard::Unknown)
		SELF_STR_CASE(sf::Keyboard::A)
		SELF_STR_CASE(sf::Keyboard::B)
		SELF_STR_CASE(sf::Keyboard::C)
		SELF_STR_CASE(sf::Keyboard::D)
		SELF_STR_CASE(sf::Keyboard::E)
		SELF_STR_CASE(sf::Keyboard::F)
		SELF_STR_CASE(sf::Keyboard::G)
		SELF_STR_CASE(sf::Keyboard::H)
		SELF_STR_CASE(sf::Keyboard::I)
		SELF_STR_CASE(sf::Keyboard::J)
		SELF_STR_CASE(sf::Keyboard::K)
		SELF_STR_CASE(sf::Keyboard::L)
		SELF_STR_CASE(sf::Keyboard::M)
		SELF_STR_CASE(sf::Keyboard::N)
		SELF_STR_CASE(sf::Keyboard::O)
		SELF_STR_CASE(sf::Keyboard::P)
		SELF_STR_CASE(sf::Keyboard::Q)
		SELF_STR_CASE(sf::Keyboard::R)
		SELF_STR_CASE(sf::Keyboard::S)
		SELF_STR_CASE(sf::Keyboard::T)
		SELF_STR_CASE(sf::Keyboard::U)
		SELF_STR_CASE(sf::Keyboard::V)
		SELF_STR_CASE(sf::Keyboard::W)
		SELF_STR_CASE(sf::Keyboard::X)
		SELF_STR_CASE(sf::Keyboard::Y)
		SELF_STR_CASE(sf::Keyboard::Z)
		SELF_STR_CASE(sf::Keyboard::Num0)
		SELF_STR_CASE(sf::Keyboard::Num1)
		SELF_STR_CASE(sf::Keyboard::Num2)
		SELF_STR_CASE(sf::Keyboard::Num3)
		SELF_STR_CASE(sf::Keyboard::Num4)
		SELF_STR_CASE(sf::Keyboard::Num5)
		SELF_STR_CASE(sf::Keyboard::Num6)
		SELF_STR_CASE(sf::Keyboard::Num7)
		SELF_STR_CASE(sf::Keyboard::Num8)
		SELF_STR_CASE(sf::Keyboard::Num9)
		SELF_STR_CASE(sf::Keyboard::Escape)
		SELF_STR_CASE(sf::Keyboard::LControl)
		SELF_STR_CASE(sf::Keyboard::LShift)
		SELF_STR_CASE(sf::Keyboard::LAlt)
		SELF_STR_CASE(sf::Keyboard::LSystem)
		SELF_STR_CASE(sf::Keyboard::RControl)
		SELF_STR_CASE(sf::Keyboard::RShift)
		SELF_STR_CASE(sf::Keyboard::RAlt)
		SELF_STR_CASE(sf::Keyboard::RSystem)
		SELF_STR_CASE(sf::Keyboard::Menu)
		SELF_STR_CASE(sf::Keyboard::LBracket)
		SELF_STR_CASE(sf::Keyboard::RBracket)
		SELF_STR_CASE(sf::Keyboard::SemiColon)
		SELF_STR_CASE(sf::Keyboard::Comma)
		SELF_STR_CASE(sf::Keyboard::Period)
		SELF_STR_CASE(sf::Keyboard::Quote)
		SELF_STR_CASE(sf::Keyboard::Slash)
		SELF_STR_CASE(sf::Keyboard::BackSlash)
		SELF_STR_CASE(sf::Keyboard::Tilde)
		SELF_STR_CASE(sf::Keyboard::Equal)
		SELF_STR_CASE(sf::Keyboard::Dash)
		SELF_STR_CASE(sf::Keyboard::Space)
		SELF_STR_CASE(sf::Keyboard::Return)
		SELF_STR_CASE(sf::Keyboard::BackSpace)
		SELF_STR_CASE(sf::Keyboard::Tab)
		SELF_STR_CASE(sf::Keyboard::PageUp)
		SELF_STR_CASE(sf::Keyboard::PageDown)
		SELF_STR_CASE(sf::Keyboard::End)
		SELF_STR_CASE(sf::Keyboard::Home)
		SELF_STR_CASE(sf::Keyboard::Insert)
		SELF_STR_CASE(sf::Keyboard::Delete)
		SELF_STR_CASE(sf::Keyboard::Add)
		SELF_STR_CASE(sf::Keyboard::Subtract)
		SELF_STR_CASE(sf::Keyboard::Multiply)
		SELF_STR_CASE(sf::Keyboard::Divide)
		SELF_STR_CASE(sf::Keyboard::Left)
		SELF_STR_CASE(sf::Keyboard::Right)
		SELF_STR_CASE(sf::Keyboard::Up)
		SELF_STR_CASE(sf::Keyboard::Down)
		SELF_STR_CASE(sf::Keyboard::Numpad0)
		SELF_STR_CASE(sf::Keyboard::Numpad1)
		SELF_STR_CASE(sf::Keyboard::Numpad2)
		SELF_STR_CASE(sf::Keyboard::Numpad3)
		SELF_STR_CASE(sf::Keyboard::Numpad4)
		SELF_STR_CASE(sf::Keyboard::Numpad5)
		SELF_STR_CASE(sf::Keyboard::Numpad6)
		SELF_STR_CASE(sf::Keyboard::Numpad7)
		SELF_STR_CASE(sf::Keyboard::Numpad8)
		SELF_STR_CASE(sf::Keyboard::Numpad9)
		SELF_STR_CASE(sf::Keyboard::F1)
		SELF_STR_CASE(sf::Keyboard::F2)
		SELF_STR_CASE(sf::Keyboard::F3)
		SELF_STR_CASE(sf::Keyboard::F4)
		SELF_STR_CASE(sf::Keyboard::F5)
		SELF_STR_CASE(sf::Keyboard::F6)
		SELF_STR_CASE(sf::Keyboard::F7)
		SELF_STR_CASE(sf::Keyboard::F8)
		SELF_STR_CASE(sf::Keyboard::F9)
		SELF_STR_CASE(sf::Keyboard::F10)
		SELF_STR_CASE(sf::Keyboard::F11)
		SELF_STR_CASE(sf::Keyboard::F12)
		SELF_STR_CASE(sf::Keyboard::F13)
		SELF_STR_CASE(sf::Keyboard::F14)
		SELF_STR_CASE(sf::Keyboard::F15)
		SELF_STR_CASE(sf::Keyboard::Pause)
		SELF_STR_CASE(sf::Keyboard::KeyCount)
		default:
		{
			return "<unknown>";
		}
	}
}

const char *GenericEngineRunner::SFMLMouseButtonEnumToString(sf::Mouse::Button button)
{
	switch (button)
	{
		SELF_STR_CASE(sf::Mouse::Left)
		SELF_STR_CASE(sf::Mouse::Right)
		SELF_STR_CASE(sf::Mouse::Middle)
		SELF_STR_CASE(sf::Mouse::XButton1)
		SELF_STR_CASE(sf::Mouse::XButton2)
		SELF_STR_CASE(sf::Mouse::ButtonCount)
		default:
		{
			return "<unknown>";
		}
	}
}

#undef SELF_STR_CASE

