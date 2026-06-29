#include "CaveCarver.h"

#include "Maths/MiscMath.h"
#include "Maths/Converter.h"
#include "Generator/Noise/PerlinNoise.h"

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

	// Where caves cluster densely, let tunnels reach the surface so the system has mouths that lead
	// down into the deep network, instead of staying sealed under the cover.
	if (col.region > REGION_OPEN)
		col.spaghettiCover = 0;

	lakeAt(xz, col);

	// Ravines: land only, so canyons never open a dry slot under the sea, and kept rare by the mask.
	if (surfaceHeight > Const::SEA_LEVEL + RAVINE_LAND_MARGIN && m_ravineMask.getNoise(p) >= RAVINE_MASK_T) {
		col.ravine = true;
		col.ravineTop = surfaceHeight - 3;
		col.ravineDist = std::abs(m_ravinePath.getNoise(p));
	}
	return col;
}

int CaveCarver::hash(int x, int z, int salt) {
	int h = PerlinNoise::perm[posMod(x + salt * 71, 256)];
	h = PerlinNoise::perm[posMod(h + z + salt * 131, 256)];
	return PerlinNoise::perm[posMod(h + salt * 197, 256)];
}

void CaveCarver::lakeAt(ivec2 xz, Column& col) const {
	// Warp the cell grid so region edges curve. The warp is smooth, so we sample it once here and
	// reuse it for the nearby seam lookups rather than re-evaluating noise for each.
	dvec2 p{ double(xz.x), double(xz.y) };
	double ox = m_lakeWarp.getNoise(p) * LAKE_WARP;
	double oz = m_lakeWarp.getNoise(p + dvec2{ 71.3, 19.7 }) * LAKE_WARP;
	auto levelAt = [&](int dx, int dz) {
		ivec2 cell{ floorDiv(int(std::floor(xz.x + dx + ox)), LAKE_CELL),
			floorDiv(int(std::floor(xz.y + dz + oz)), LAKE_CELL) };
		if (hash(cell.x, cell.y, 1) >= LAKE_FILL)
			return 0; // dry region: no lava, so caves here stay open
		return Const::LAVA_MIN + hash(cell.x, cell.y, 0) * (Const::LAVA_MAX - Const::LAVA_MIN) / 255;
	};

	int level = levelAt(0, 0);
	col.lavaLevel = level;

	// Look a wall's width out in eight directions. If a different region is that close, this column
	// is on the seam: keep it solid up to the higher level so the two pools stay flat and separate.
	int hi = level;
	bool seam = false;
	const ivec2 dirs[8]{ { LAKE_WALL, 0 }, { -LAKE_WALL, 0 }, { 0, LAKE_WALL }, { 0, -LAKE_WALL },
		{ LAKE_WALL, LAKE_WALL }, { LAKE_WALL, -LAKE_WALL }, { -LAKE_WALL, LAKE_WALL }, { -LAKE_WALL, -LAKE_WALL } };
	for (const ivec2& d : dirs) {
		int n = levelAt(d.x, d.y);
		if (n != level) {
			seam = true;
			hi = std::max(hi, n);
		}
	}
	if (seam)
		col.wallLevel = hi;
}

bool CaveCarver::ravineCarved(const Column& col, int y) const {
	if (!col.ravine || y < RAVINE_FLOOR || y > col.ravineTop)
		return false;
	// Narrow at the floor, wide at the top, for a classic canyon profile.
	double t = double(y - RAVINE_FLOOR) / double(col.ravineTop - RAVINE_FLOOR);
	return col.ravineDist < RAVINE_HALF * (0.25 + 0.75 * t);
}

bool CaveCarver::isCarved(const Grid& grid, const Column& col, ivec3 local, int depth) const {
	if (local.y < FLOOR_Y)
		return false;
	// On a seam between two lava heights, keep rock solid so the neighbouring pools stay separate.
	if (local.y < col.wallLevel)
		return false;
	if (ravineCarved(col, local.y))
		return true;
	if (col.region <= 0. || local.y > grid.maxY)
		return false;

	bool belowSea = local.y < Const::SEA_LEVEL;
	ivec3 cell{ local.x / LXZ, local.y / LY, local.z / LXZ };
	dvec3 frac{ (local.x % LXZ) / double(LXZ), (local.y % LY) / double(LY), (local.z % LXZ) / double(LXZ) };

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
