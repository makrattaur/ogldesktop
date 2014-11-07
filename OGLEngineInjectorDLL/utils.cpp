#include "utils.h"


#define SELF_STR_CASE(x) case x: \
{ \
	return #x; \
}

const char *SFMLEventTypeEnumToString(sf::Event::EventType etype)
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

#undef SELF_STR_CASE
