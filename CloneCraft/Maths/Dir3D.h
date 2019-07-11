#pragma once

#include "Maths/GlmCommon.h"

#include <array>

class Dir3D {
public:
	static const int SIZE = 6;
	enum Dir { UP = 0, FRONT, RIGHT, DOWN, BACK, LEFT };

	Dir3D() = delete;
	static std::array<Dir3D::Dir, SIZE> all();
	static ivec3 to_ivec3(Dir dir);
	static Dir prev(Dir dir);
	static Dir next(Dir dir);
	static Dir opp(Dir dir);
	static std::array<Dir3D::Dir, 4> all_horizontal();

private:
	static const std::array<Dir3D::Dir, SIZE> s_all;
	static const std::array<ivec3, SIZE> s_all_dirs;
};

