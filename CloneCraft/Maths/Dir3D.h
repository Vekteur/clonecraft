#pragma once

#include "GlmCommon.h"

#include <array>

class Dir3D {
public:
	static const int SIZE = 6;
	enum Dir { UP = 0, FRONT, RIGHT, DOWN, BACK, LEFT };

	Dir3D() = delete;
	static std::array<Dir3D::Dir, SIZE> all();
	static std::array<ivec3, SIZE> all_dirs();
	static ivec3 find(Dir dir);
	static ivec3 prev(Dir dir);
	static ivec3 next(Dir dir);
	static ivec3 oppDir(Dir dir);
	static std::array<Dir3D::Dir, 4> horizontal();

private:
	static const std::array<Dir3D::Dir, SIZE> s_all;
	static const std::array<ivec3, SIZE> s_all_dirs;
};

