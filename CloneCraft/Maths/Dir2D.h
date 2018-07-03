#pragma once

#include "GlmCommon.h"

#include <array>

class Dir2D {
public:
	static const int SIZE = 4;
	enum Dir {UP = 0, RIGHT, DOWN, LEFT};

	Dir2D() = delete;
	static std::array<ivec2, SIZE> all_dirs();
	static ivec2 find(Dir dir);
	static ivec2 prev(Dir dir);
	static ivec2 next(Dir dir);
	static ivec2 oppDir(Dir dir);

private:
	static const std::array<ivec2, SIZE> dirs;
};

