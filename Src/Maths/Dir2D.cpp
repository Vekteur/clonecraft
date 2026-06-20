#include "Dir2D.h"

namespace Dir2D {
	const std::array<Dir, SIZE> s_all
	{ { FRONT, RIGHT, BACK, LEFT } };

	const std::array<ivec2, SIZE> s_all_dirs{ { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } } };

	std::array<Dir, SIZE> all() {
		return s_all;
	}

	ivec2 to_ivec2(Dir dir) {
		return s_all_dirs[dir];
	}

	Dir prev(Dir dir) {
		return static_cast<Dir>((dir - 1 + SIZE) % SIZE);
	}

	Dir next(Dir dir) {
		return static_cast<Dir>((dir + 1) % SIZE);
	}

	Dir opp(Dir dir) {
		return static_cast<Dir>((dir + SIZE / 2) % SIZE);
	}
}