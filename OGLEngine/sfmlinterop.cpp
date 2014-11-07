#include "sfmlinterop.h"


KeyboardKey SFMLInterop::ConvertKeyboardKey(sf::Keyboard::Key key)
{
	switch (key)
	{
		case sf::Keyboard::Unknown:     return KeyboardKey::UnknownKey;
		case sf::Keyboard::A:           return KeyboardKey::A;
		case sf::Keyboard::B:           return KeyboardKey::B;
		case sf::Keyboard::C:           return KeyboardKey::C;
		case sf::Keyboard::D:           return KeyboardKey::D;
		case sf::Keyboard::E:           return KeyboardKey::E;
		case sf::Keyboard::F:           return KeyboardKey::F;
		case sf::Keyboard::G:           return KeyboardKey::G;
		case sf::Keyboard::H:           return KeyboardKey::H;
		case sf::Keyboard::I:           return KeyboardKey::I;
		case sf::Keyboard::J:           return KeyboardKey::J;
		case sf::Keyboard::K:           return KeyboardKey::K;
		case sf::Keyboard::L:           return KeyboardKey::L;
		case sf::Keyboard::M:           return KeyboardKey::M;
		case sf::Keyboard::N:           return KeyboardKey::N;
		case sf::Keyboard::O:           return KeyboardKey::O;
		case sf::Keyboard::P:           return KeyboardKey::P;
		case sf::Keyboard::Q:           return KeyboardKey::Q;
		case sf::Keyboard::R:           return KeyboardKey::R;
		case sf::Keyboard::S:           return KeyboardKey::S;
		case sf::Keyboard::T:           return KeyboardKey::T;
		case sf::Keyboard::U:           return KeyboardKey::U;
		case sf::Keyboard::V:           return KeyboardKey::V;
		case sf::Keyboard::W:           return KeyboardKey::W;
		case sf::Keyboard::X:           return KeyboardKey::X;
		case sf::Keyboard::Y:           return KeyboardKey::Y;
		case sf::Keyboard::Z:           return KeyboardKey::Z;
		case sf::Keyboard::Num0:        return KeyboardKey::Num0;
		case sf::Keyboard::Num1:        return KeyboardKey::Num1;
		case sf::Keyboard::Num2:        return KeyboardKey::Num2;
		case sf::Keyboard::Num3:        return KeyboardKey::Num3;
		case sf::Keyboard::Num4:        return KeyboardKey::Num4;
		case sf::Keyboard::Num5:        return KeyboardKey::Num5;
		case sf::Keyboard::Num6:        return KeyboardKey::Num6;
		case sf::Keyboard::Num7:        return KeyboardKey::Num7;
		case sf::Keyboard::Num8:        return KeyboardKey::Num8;
		case sf::Keyboard::Num9:        return KeyboardKey::Num9;
		case sf::Keyboard::Escape:      return KeyboardKey::Esc;
		case sf::Keyboard::LControl:    return KeyboardKey::LCtrl;
		case sf::Keyboard::LShift:      return KeyboardKey::LShift;
		case sf::Keyboard::LAlt:        return KeyboardKey::LAlt;
		case sf::Keyboard::LSystem:     return KeyboardKey::LWin;
		case sf::Keyboard::RControl:    return KeyboardKey::RCtrl;
		case sf::Keyboard::RShift:      return KeyboardKey::RShift;
		case sf::Keyboard::RAlt:        return KeyboardKey::RAlt;
		case sf::Keyboard::RSystem:     return KeyboardKey::RWin;
		case sf::Keyboard::Menu:        return KeyboardKey::App;
		case sf::Keyboard::LBracket:    return KeyboardKey::LBracket;
		case sf::Keyboard::RBracket:    return KeyboardKey::RBracket;
		case sf::Keyboard::SemiColon:   return KeyboardKey::SemiColon;
		case sf::Keyboard::Comma:       return KeyboardKey::Comma;
		case sf::Keyboard::Period:      return KeyboardKey::Period;
		case sf::Keyboard::Quote:       return KeyboardKey::Quote;
		case sf::Keyboard::Slash:       return KeyboardKey::Divide;
		case sf::Keyboard::BackSlash:   return KeyboardKey::BackSlash;
		case sf::Keyboard::Tilde:       return KeyboardKey::Tilde;
		case sf::Keyboard::Equal:       return KeyboardKey::Equal;
		case sf::Keyboard::Dash:        return KeyboardKey::Minus;
		case sf::Keyboard::Space:       return KeyboardKey::Space;
		case sf::Keyboard::Return:      return KeyboardKey::Enter;
		case sf::Keyboard::BackSpace:   return KeyboardKey::BackSpace;
		case sf::Keyboard::Tab:         return KeyboardKey::Tab;
		case sf::Keyboard::PageUp:      return KeyboardKey::PageUp;
		case sf::Keyboard::PageDown:    return KeyboardKey::PageDown;
		case sf::Keyboard::End:         return KeyboardKey::End;
		case sf::Keyboard::Home:        return KeyboardKey::Home;
		case sf::Keyboard::Insert:      return KeyboardKey::Insert;
		case sf::Keyboard::Delete:      return KeyboardKey::Delete;
		case sf::Keyboard::Add:         return KeyboardKey::Plus;
		case sf::Keyboard::Subtract:    return KeyboardKey::Minus;
		case sf::Keyboard::Multiply:    return KeyboardKey::Multiply;
		case sf::Keyboard::Divide:      return KeyboardKey::Divide;
		case sf::Keyboard::Left:        return KeyboardKey::LeftArrow;
		case sf::Keyboard::Right:       return KeyboardKey::RightArrow;
		case sf::Keyboard::Up:          return KeyboardKey::UpArrow;
		case sf::Keyboard::Down:        return KeyboardKey::DownArrow;
		case sf::Keyboard::Numpad0:     return KeyboardKey::Numpad0;
		case sf::Keyboard::Numpad1:     return KeyboardKey::Numpad1;
		case sf::Keyboard::Numpad2:     return KeyboardKey::Numpad2;
		case sf::Keyboard::Numpad3:     return KeyboardKey::Numpad3;
		case sf::Keyboard::Numpad4:     return KeyboardKey::Numpad4;
		case sf::Keyboard::Numpad5:     return KeyboardKey::Numpad5;
		case sf::Keyboard::Numpad6:     return KeyboardKey::Numpad6;
		case sf::Keyboard::Numpad7:     return KeyboardKey::Numpad7;
		case sf::Keyboard::Numpad8:     return KeyboardKey::Numpad8;
		case sf::Keyboard::Numpad9:     return KeyboardKey::Numpad9;
		case sf::Keyboard::F1:          return KeyboardKey::F1;
		case sf::Keyboard::F2:          return KeyboardKey::F2;
		case sf::Keyboard::F3:          return KeyboardKey::F3;
		case sf::Keyboard::F4:          return KeyboardKey::F4;
		case sf::Keyboard::F5:          return KeyboardKey::F5;
		case sf::Keyboard::F6:          return KeyboardKey::F6;
		case sf::Keyboard::F7:          return KeyboardKey::F7;
		case sf::Keyboard::F8:          return KeyboardKey::F8;
		case sf::Keyboard::F9:          return KeyboardKey::F9;
		case sf::Keyboard::F10:         return KeyboardKey::F10;
		case sf::Keyboard::F11:         return KeyboardKey::F11;
		case sf::Keyboard::F12:         return KeyboardKey::F12;
		case sf::Keyboard::Pause:       return KeyboardKey::Pause;
		default:                        return KeyboardKey::UnknownKey;
	}
}

MouseButton SFMLInterop::ConvertMouseButton(sf::Mouse::Button button)
{
	switch (button)
	{
		case sf::Mouse::Left:	  return MouseButton::Left;
		case sf::Mouse::Right:	  return MouseButton::Right;
		case sf::Mouse::Middle:	  return MouseButton::Middle;
		case sf::Mouse::XButton1: return MouseButton::XButton1;
		case sf::Mouse::XButton2: return MouseButton::XButton2;
	}

	return MouseButton::Left; // !!
}

