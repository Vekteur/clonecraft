#include "CaveCarver.h"

#include "Maths/MiscMath.h"

#include <algorithm>
#include <cmath>

CaveCarver::Grid::Grid(int nx, int ny, int nz, int maxY)
	: maxY{ maxY }, spaghettiA({ nx, ny, nz }), spaghettiB({ nx, ny, nz }), cheese({ nx, ny, nz }) { }

CaveCarver::Grid CaveCarver::sample(ivec3 origin, int maxY) const {
	int nx = Const::SECTION_SIDE / LXZ + 1, nz = Const::SECTION_SIDE / LXZ + 1, ny = maxY / LY + 2;
	Grid g(nx, ny, nz, maxY);
	for (int i = 0; i < nx; ++i)
		for (int k = 0; k < nz; ++k)
			for (int j = 0; j < ny; ++j) {
				double wx = origin.x + i * LXZ, wz = origin.z + k * LXZ, wy = j * LY;
				dvec3 ps{ wx, wy * SPAGHETTI_V, wz };
				g.spaghettiA.at({ i, j, k }) = m_spaghetti.getNoise(ps);
				g.spaghettiB.at({ i, j, k }) = m_spaghetti.getNoise(ps + SPAGHETTI_OFFSET);
				g.cheese.at({ i, j, k }) = m_cheese.getNoise(dvec3{ wx, wy * CHEESE_V, wz });
			}
	return g;
}

CaveCarver::Column CaveCarver::column(ivec2 xz, int surfaceHeight) const {
	dvec2 p{ double(xz.x), double(xz.y) };
	Column col;
	double region = math::smoothstep((m_region.getNoise(p) - REGION_T) / REGION_FADE);

	// Taller terrain is more cave-prone and lets tunnels break out onto the cliffs.
	double mountainness = math::smoothstep((surfaceHeight - MOUNTAIN_START) / double(MOUNTAIN_RANGE));
	col.region = std::max(region, mountainness);
	col.spaghettiCover = int(std::round(math::lerp(mountainness, SPAGHETTI_LAND_COVER, MOUNTAIN_SPAGHETTI_COVER)));

	// Ravines: land only, so canyons never open a dry slot under the sea, and kept rare by the mask.
	if (surfaceHeight > Const::SEA_LEVEL + RAVINE_LAND_MARGIN && m_ravineMask.getNoise(p) >= RAVINE_MASK_T) {
		col.ravine = true;
		col.ravineTop = surfaceHeight - 3;
		col.ravineDist = std::abs(m_ravinePath.getNoise(p));
	}
	return col;
}

bool CaveCarver::ravineCarved(const Column& col, int y) const {
	if (!col.ravine || y < RAVINE_FLOOR || y > col.ravineTop)
		return false;
	// Narrow at the floor, wide at the top, for a classic canyon profile.
	double t = double(y - RAVINE_FLOOR) / double(col.ravineTop - RAVINE_FLOOR);
	return col.ravineDist < RAVINE_HALF * (0.25 + 0.75 * t);
}

bool CaveCarver::isCarved(const Grid& grid, const Column& col, ivec3 local, ivec3 pos, int depth) const {
	if (pos.y < FLOOR_Y)
		return false;
	if (ravineCarved(col, pos.y))
		return true;
	if (col.region <= 0. || pos.y > grid.maxY)
		return false;

	bool belowSea = pos.y < Const::SEA_LEVEL;
	ivec3 cell{ local.x / LXZ, pos.y / LY, local.z / LXZ };
	dvec3 frac{ (local.x % LXZ) / double(LXZ), (pos.y % LY) / double(LY), (local.z % LXZ) / double(LXZ) };

	// Spaghetti: a round tube where both fields are near zero, kept under the surface.
	int spaghettiCover = belowSea ? SUBMERGED_COVER : col.spaghettiCover;
	if (depth >= spaghettiCover) {
		double a = math::trilerp(grid.spaghettiA, cell, frac);
		double b = math::trilerp(grid.spaghettiB, cell, frac);
		double r = SPAGHETTI_R * col.region;
		if (a * a + b * b < r * r)
			return true;
	}

	// Cheese: big caverns, allowed to reach the surface on land for natural mouths.
	int cheeseCover = belowSea ? SUBMERGED_COVER : CHEESE_LAND_COVER;
	if (depth >= cheeseCover) {
		double threshold = CHEESE_T + (1. - col.region) * 0.3;
		if (math::trilerp(grid.cheese, cell, frac) > threshold)
			return true;
	}

	return false;
}
