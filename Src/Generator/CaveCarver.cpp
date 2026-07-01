#include "CaveCarver.h"

#include "Maths/MiscMath.h"
#include "Maths/Converter.h"
#include "Generator/Noise/PerlinNoise.h"

#include <algorithm>
#include <cmath>

CaveCarver::Grid::Grid(int nx, int ny, int nz, int maxY)
	: maxY{ maxY }, spaghettiA({ nx, ny, nz }), spaghettiB({ nx, ny, nz }), cheese({ nx, ny, nz }) { }

CaveCarver::Grid CaveCarver::sample(ivec3 origin, int maxY) const {
	int nx = Const::SECTION_SIDE / L.x + 1, nz = Const::SECTION_SIDE / L.z + 1, ny = maxY / L.y + 2;
	Grid g(nx, ny, nz, maxY);
	for (int i = 0; i < nx; ++i)
		for (int k = 0; k < nz; ++k)
			for (int j = 0; j < ny; ++j) {
				const ivec3 d{ i, j, k };
				dvec3 w{ origin + d * L };
				dvec3 ps{ w.x, w.y * SPAGHETTI_V, w.z };
				g.spaghettiA.at(d) = m_spaghetti.getNoise(ps);
				g.spaghettiB.at(d) = m_spaghetti.getNoise(ps + SPAGHETTI_OFFSET);
				g.cheese.at(d) = m_cheese.getNoise({ w.x, w.y * CHEESE_V, w.z });
			}
	return g;
}

CaveCarver::Column CaveCarver::column(ivec2 xz, int surfaceHeight) const {
	dvec2 p(xz);
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

int CaveCarver::hash(ivec2 cell, int salt) {
	int h = PerlinNoise::perm[posMod(cell.x + salt * 71, 256)];
	h = PerlinNoise::perm[posMod(h + cell.y + salt * 131, 256)];
	return PerlinNoise::perm[posMod(h + salt * 197, 256)];
}

void CaveCarver::lakeAt(ivec2 xz, Column& col) const {
	// Warp the cell grid so region edges curve. The warp is smooth, so we sample it once here and
	// reuse it for the nearby seam lookups rather than re-evaluating noise for each.
	dvec2 p(xz);
	dvec2 warp{ m_lakeWarp.getNoise(p) * LAKE_WARP, m_lakeWarp.getNoise(p + dvec2{ 71.3, 19.7 }) * LAKE_WARP };
	auto levelAt = [&](ivec2 d) {
		ivec2 cell = floorDiv(ivec2(glm::floor(dvec2(xz + d) + warp)), ivec2(LAKE_CELL));
		if (hash(cell, 1) >= LAKE_FILL)
			return 0; // dry region: no lava, so caves here stay open
		return Const::LAVA_MIN + hash(cell, 0) * (Const::LAVA_MAX - Const::LAVA_MIN) / 255;
	};

	int level = levelAt({ 0, 0 });
	col.lavaLevel = level;

	// Look a wall's width out in eight directions. If a different region is that close, this column
	// is on the seam: keep it solid up to the higher level so the two pools stay flat and separate.
	int hi = level;
	bool seam = false;
	const ivec2 dirs[8]{ { LAKE_WALL, 0 }, { -LAKE_WALL, 0 }, { 0, LAKE_WALL }, { 0, -LAKE_WALL },
		{ LAKE_WALL, LAKE_WALL }, { LAKE_WALL, -LAKE_WALL }, { -LAKE_WALL, LAKE_WALL }, { -LAKE_WALL, -LAKE_WALL } };
	for (const ivec2& d : dirs) {
		int n = levelAt(d);
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

// The carving decision, shared by both callers. The three fields differ only in how they are sampled
// (interpolated from a pre-built grid, or rebuilt from noise on the fly), so each caller passes the
// samplers and this holds the rest. A sampler is invoked only when its cover lets it carve, so a field
// is never sampled when it cannot change the result.
template<typename SampleSpaghetti, typename SampleCheese>
bool CaveCarver::carved(const Column& col, int y, int depth,
		SampleSpaghetti sampleSpaghetti, SampleCheese sampleCheese) const {
	if (y < FLOOR_Y)
		return false;
	// On a seam between two lava heights, keep rock solid so the neighbouring pools stay separate.
	if (y < col.wallLevel)
		return false;
	if (ravineCarved(col, y))
		return true;
	if (col.region <= 0.)
		return false;

	bool belowSea = y < Const::SEA_LEVEL;

	// Spaghetti: a round tube where both fields are near zero, kept under the surface.
	int spaghettiCover = belowSea ? SUBMERGED_COVER : col.spaghettiCover;
	if (depth >= spaghettiCover) {
		dvec2 s = sampleSpaghetti();
		double r = SPAGHETTI_R * col.region;
		if (s.x * s.x + s.y * s.y < r * r)
			return true;
	}

	// Cheese: big caverns, allowed to reach the surface on land for natural mouths.
	int cheeseCover = belowSea ? SUBMERGED_COVER : CHEESE_LAND_COVER;
	if (depth >= cheeseCover) {
		double threshold = CHEESE_T + (1. - col.region) * 0.3;
		if (sampleCheese() > threshold)
			return true;
	}

	return false;
}

template<typename Noise>
double CaveCarver::latticeSample(ivec3 pos, Noise noiseAt) const {
	math::LatticeCoord lc = math::latticeCoord(pos, L);
	DynamicArray3D<double> corners({ 2, 2, 2 });
	for (int dx = 0; dx < 2; ++dx)
		for (int dy = 0; dy < 2; ++dy)
			for (int dz = 0; dz < 2; ++dz) {
				const ivec3 d{ dx, dy, dz };
				corners.at(d) = noiseAt((lc.cell + d) * L);
			}
	return math::trilerp(corners, { 0, 0, 0 }, lc.frac);
}

bool CaveCarver::isCarved(const Grid& grid, const Column& col, ivec3 local, int depth) const {
	if (local.y > grid.maxY) // above the sampled region; a ravine never reaches this high
		return false;
	math::LatticeCoord lc = math::latticeCoord(local, L);
	return carved(col, local.y, depth,
		[&]() {
			double s1 = math::trilerp(grid.spaghettiA, lc.cell, lc.frac);
			double s2 = math::trilerp(grid.spaghettiB, lc.cell, lc.frac);
			return dvec2{ s1, s2 };
		},
		[&] { return math::trilerp(grid.cheese, lc.cell, lc.frac); });
}

bool CaveCarver::isCarvedPointwise(const Column& col, ivec3 pos, int depth) const {
	return carved(col, pos.y, depth,
		[&]() {
			double s1 = latticeSample(pos, [&](ivec3 node) {
				return m_spaghetti.getNoise(dvec3{ double(node.x), node.y * SPAGHETTI_V, double(node.z) });
			});
			double s2 = latticeSample(pos, [&](ivec3 node) {
				return m_spaghetti.getNoise(dvec3{ double(node.x), node.y * SPAGHETTI_V, double(node.z) } + SPAGHETTI_OFFSET);
			});
			return dvec2{ s1, s2 };
		},
		[&] {
			return latticeSample(pos, [&](ivec3 node) {
				return m_cheese.getNoise(dvec3{ double(node.x), node.y * CHEESE_V, double(node.z) });
			});
		});
}
