#include "Dir3D.h"


std::array<ivec3, Dir3D::SIZE> Dir3D::all_dirs() {
	return dirs;
}

ivec3 Dir3D::find(Dir dir) {
	return dirs[dir];
}

ivec3 Dir3D::prev(Dir dir) {
	return dirs[(dir - 1 + SIZE) % SIZE];
}

ivec3 Dir3D::next(Dir dir) {
	return dirs[(dir + 1) % SIZE];
}

ivec3 Dir3D::oppDir(Dir dir) {
	return dirs[(dir + SIZE / 2) % SIZE];
}

const std::array<ivec3, Dir3D::SIZE> Dir3D::dirs{ { { 0, 1, 0 }, { 1, 0, 0 }, { 0, 0, 1 }, { 0, -1, 0 }, { -1, 0, 0 }, { 0, 0, -1 } } };