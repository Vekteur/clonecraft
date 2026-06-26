#pragma once

#include "Maths/GlmCommon.h"

#include <array>
#include <optional>

namespace Dir2D {
	enum Dir : int;
}

namespace Dir3D {
	const int SIZE = 6;
	enum Dir { UP = 0, FRONT, RIGHT, DOWN, BACK, LEFT };

	std::array<Dir3D::Dir, SIZE> all();
	ivec3 to_ivec3(Dir dir);
	Dir from_ivec3(ivec3 v);
	Dir prev(Dir dir);
	Dir next(Dir dir);
	Dir opp(Dir dir);
	std::array<Dir3D::Dir, 4> all_horizontal();
	std::optional<Dir2D::Dir> to_2D(Dir dir);
};

