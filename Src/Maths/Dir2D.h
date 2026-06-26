#pragma once

#include "Maths/GlmCommon.h"

#include <array>

namespace Dir2D {
	const int SIZE = 4;
	enum Dir : int {FRONT = 0, RIGHT, BACK, LEFT};

	std::array<Dir, SIZE> all();
	ivec2 to_ivec2(Dir dir);
	Dir from_ivec2(ivec2 v);
	Dir prev(Dir dir);
	Dir next(Dir dir);
	Dir opp(Dir dir);
};
