#include "Dir2D.h"

std::array<Dir2D::Dir, Dir2D::SIZE> Dir2D::all() {
	return s_all;
}

ivec2 Dir2D::to_ivec2(Dir dir) {
	return s_all_dirs[dir];
}

Dir2D::Dir Dir2D::prev(Dir dir) {
	return static_cast<Dir2D::Dir>((dir - 1 + SIZE) % SIZE);
}

Dir2D::Dir Dir2D::next(Dir dir) {
	return static_cast<Dir2D::Dir>((dir + 1) % SIZE);
}

Dir2D::Dir Dir2D::opp(Dir dir) {
	return static_cast<Dir2D::Dir>((dir + SIZE / 2) % SIZE);
}

const std::array<Dir2D::Dir, Dir2D::SIZE> Dir2D::s_all
{ { Dir2D::UP, Dir2D::RIGHT, Dir2D::DOWN, Dir2D::LEFT } };

const std::array<ivec2, Dir2D::SIZE> Dir2D::s_all_dirs{ { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } } };