#include "Keyboard.h"

bool Keyboard::s_keys[1024]{};

void Keyboard::setKey(int key, bool enable)
{
	s_keys[key] = enable;
}

bool Keyboard::isKeyPressed(int key)
{
	return s_keys[key];
}
