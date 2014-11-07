#ifndef G_H_SFMLINTEROP
#define G_H_SFMLINTEROP

#include "enginecommon.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include "input.h"


class ENGINE_API SFMLInterop
{
public:
	static KeyboardKey ConvertKeyboardKey(sf::Keyboard::Key key);
	static MouseButton ConvertMouseButton(sf::Mouse::Button button);
};

#endif

