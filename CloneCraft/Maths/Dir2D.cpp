#include "Dir2D.h"


std::array<ivec2, Dir2D::SIZE> Dir2D::all_dirs() {
	return dirs;
}

ivec2 Dir2D::find(Dir dir) {
	return dirs[dir];
}

ivec2 Dir2D::prev(Dir dir) {
	return dirs[(dir - 1 + SIZE) % SIZE];
}

ivec2 Dir2D::next(Dir dir) {
	return dirs[(dir + 1) % SIZE];
}

ivec2 Dir2D::oppDir(Dir dir) {
	return dirs[(dir + SIZE / 2) % SIZE];
}

const std::array<ivec2, Dir2D::SIZE> Dir2D::dirs{ { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } } };