#include "Frustum.h"

#include "Logger.h"

/*
The equations of the frustum planes are determined from the projection and view matrices
Algorithm based on this article : 
http://gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
*/
Frustum::Frustum(mat4 projView) {
	for (int dir = 0; dir < Dir3D::SIZE; ++dir) {
		float factor = ((dir & 1) ? 1 : -1);
		for (int i = 0; i < 4; ++i) {
			planes[dir][i] = projView[i][3] + projView[i][dir / 2] * factor;
		}
	}
	for (Plane& plane : planes) {
		plane.normalize();
	}
}

bool Frustum::isBoxOutside(Box box) const {
	for (Dir3D::Dir dir : planeDirs) {
		const Plane& plane = planes[dir];
		if (!plane.contains(box.firstPointCrossed(plane.norm()))
			&& !plane.contains(box.lastPointCrossed(plane.norm()))) {
			return true;
		}
	}
	return false;
}

const std::array<Dir3D::Dir, Dir3D::SIZE> Frustum::planeDirs
{ Dir3D::RIGHT, Dir3D::LEFT, Dir3D::DOWN, Dir3D::UP, Dir3D::FRONT, Dir3D::BACK };
