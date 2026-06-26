#include "Dir3D.h"

#include "Dir2D.h"


namespace Dir3D {
	const std::array<Dir, SIZE> s_all
	{ { UP, FRONT, RIGHT, DOWN, BACK, LEFT } };

	const std::array<ivec3, SIZE> s_all_dirs
	{ { { 0, 1, 0 }, { 1, 0, 0 }, { 0, 0, 1 }, { 0, -1, 0 }, { -1, 0, 0 }, { 0, 0, -1 } } };

	std::array<Dir, SIZE> all() {
		return s_all;
	}

	ivec3 to_ivec3(Dir dir) {
		return s_all_dirs[dir];
	}

	Dir from_ivec3(ivec3 v) {
		if (v.y == 1) return UP;
		if (v.x == 1) return FRONT;
		if (v.z == 1) return RIGHT;
		if (v.y == -1) return DOWN;
		if (v.x == -1) return BACK;
		if (v.z == -1) return LEFT;
		throw std::invalid_argument("Invalid 3D direction vector");
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

	std::array<Dir, 4> all_horizontal() {
		return { FRONT, RIGHT, BACK, LEFT };
	}

	std::optional<Dir2D::Dir> to_2D(Dir dir) {
		switch(dir) {
			case FRONT: return Dir2D::FRONT;
			case BACK: return Dir2D::BACK;
			case LEFT: return Dir2D::LEFT;
			case RIGHT: return Dir2D::RIGHT;
			default: return std::nullopt;
		}
	}
}