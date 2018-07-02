#pragma once

#include <GlmCommon.h>

class Mouse {
public:
	Mouse() = delete;

	static void setPosition(vec2 position);
	static vec2 getPosition();
	static void addScrolling(vec2 scrolling);
	static void resetScrolling();
	static vec2 getScrolling();

private:
	static vec2 s_position;
	static vec2 s_scrolling;
};
