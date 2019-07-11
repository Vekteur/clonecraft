#pragma once

#include "Maths/GlmCommon.h"

#include <array>

class Dir2D {
public:
	static const int SIZE = 4;
	enum Dir {UP = 0, RIGHT, DOWN, LEFT};

	Dir2D() = delete;
	static std::array<Dir, SIZE> all();
	static ivec2 to_ivec2(Dir dir);
	static Dir prev(Dir dir);
	static Dir next(Dir dir);
	static Dir opp(Dir dir);

private:
	static const std::array<Dir2D::Dir, SIZE> s_all;
	static const std::array<ivec2, SIZE> s_all_dirs;
};

