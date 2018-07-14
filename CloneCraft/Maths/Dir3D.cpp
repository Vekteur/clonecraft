#include "Dir3D.h"


std::array<Dir3D::Dir, Dir3D::SIZE> Dir3D::all() {
	return s_all;
}

std::array<ivec3, Dir3D::SIZE> Dir3D::all_dirs() {
	return s_all_dirs;
}

ivec3 Dir3D::find(Dir dir) {
	return s_all_dirs[dir];
}

ivec3 Dir3D::prev(Dir dir) {
	return s_all_dirs[(dir - 1 + SIZE) % SIZE];
}

ivec3 Dir3D::next(Dir dir) {
	return s_all_dirs[(dir + 1) % SIZE];
}

ivec3 Dir3D::oppDir(Dir dir) {
	return s_all_dirs[(dir + SIZE / 2) % SIZE];
}

std::array<Dir3D::Dir, 4> Dir3D::horizontal() {
	return { Dir3D::FRONT, Dir3D::RIGHT, Dir3D::BACK, Dir3D::LEFT };
}

const std::array<Dir3D::Dir, Dir3D::SIZE> Dir3D::s_all{ { Dir3D::UP, Dir3D::FRONT, Dir3D::RIGHT, Dir3D::DOWN, Dir3D::BACK, Dir3D::LEFT } };
 
const std::array<ivec3, Dir3D::SIZE> Dir3D::s_all_dirs{ { { 0, 1, 0 }, { 1, 0, 0 }, { 0, 0, 1 }, { 0, -1, 0 }, { -1, 0, 0 }, { 0, 0, -1 } } };