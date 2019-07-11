#include "Dir3D.h"


std::array<Dir3D::Dir, Dir3D::SIZE> Dir3D::all() {
	return s_all;
}

ivec3 Dir3D::to_ivec3(Dir dir) {
	return s_all_dirs[dir];
}

Dir3D::Dir Dir3D::prev(Dir dir) {
	return static_cast<Dir3D::Dir>((dir - 1 + SIZE) % SIZE);
}

Dir3D::Dir Dir3D::next(Dir dir) {
	return static_cast<Dir3D::Dir>((dir + 1) % SIZE);
}

Dir3D::Dir Dir3D::opp(Dir dir) {
	return static_cast<Dir3D::Dir>((dir + SIZE / 2) % SIZE);
}

std::array<Dir3D::Dir, 4> Dir3D::all_horizontal() {
	return { Dir3D::FRONT, Dir3D::RIGHT, Dir3D::BACK, Dir3D::LEFT };
}

const std::array<Dir3D::Dir, Dir3D::SIZE> Dir3D::s_all
{ { Dir3D::UP, Dir3D::FRONT, Dir3D::RIGHT, Dir3D::DOWN, Dir3D::BACK, Dir3D::LEFT } };
 
const std::array<ivec3, Dir3D::SIZE> Dir3D::s_all_dirs
{ { { 0, 1, 0 }, { 1, 0, 0 }, { 0, 0, 1 }, { 0, -1, 0 }, { -1, 0, 0 }, { 0, 0, -1 } } };