#include "Mouse.h"

vec2 Mouse::s_position{ 0, 0 };
vec2 Mouse::s_scrolling{ 0, 0 };

void Mouse::setPosition(vec2 position)
{
	s_position = position;
}

vec2 Mouse::getPosition()
{
	return s_position;
}

void Mouse::addScrolling(vec2 scrolling)
{
	s_scrolling += scrolling;
}

void Mouse::resetScrolling()
{
	s_scrolling = vec2{ 0, 0 };
}

vec2 Mouse::getScrolling()
{
	return s_scrolling;
}
