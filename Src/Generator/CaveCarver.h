#pragma once

#include "Generator/Noise/OctavePerlin.h"
#include "Util/DynamicArray3D.h"
#include "World/WorldConstants.h"
#include "Maths/GlmCommon.h"

// Carves caves out of already-solid terrain. Three styles are blended:
//  - spaghetti: winding round tubes, where two 3D noise fields are both near zero
//  - cheese: large caverns from the rare peaks of a low-frequency field
//  - ravines: long vertical canyons cut along a meandering 2D line
// A low-frequency region mask thins caves out so they cluster instead of blanketing the world.
// The 3D fields are sampled on a coarse lattice per chunk and interpolated per block, like terrain.
class CaveCarver {
public:
	// Coarse-lattice samples of the 3D fields, covering one chunk up to 'maxY' (its tallest column).
	struct Grid {
		Grid(int nx, int ny, int nz, int maxY);
		int maxY;
		DynamicArray3D<double> spaghettiA, spaghettiB, cheese;
	};

	// Per-column cave data, computed once and reused down the column.
	struct Column {
		double region = 0.; // 0 = no caves in this column, 1 = full strength
		int spaghettiCover = 0; // solid blocks tunnels must stay under the surface here
		int lavaLevel = 0; // carved cells below this y fill with lava (one flat level per region)
		int wallLevel = 0; // on a region edge: keep rock solid below this y so heights don't merge
		bool ravine = false;
		int ravineTop = 0;
		double ravineDist = 0.; // distance from the ravine's center line
	};

	Grid sample(ivec3 origin, int maxY) const;
	Column column(ivec2 globalXZ, int surfaceHeight) const;

	// True if the solid block at global 'pos' should be carved to air. 'local' indexes the block
	// within the chunk (for lattice lookup), 'depth' is the solid blocks above it in the column.
	bool isCarved(const Grid& grid, const Column& col, ivec3 local, int depth) const;

private:
	static constexpr int LXZ = 4, LY = 4; // coarse lattice spacing

	// Vertical squash of each 3D field: above 1 biases tunnels/rooms horizontal.
	static constexpr double SPAGHETTI_V = 1.4, CHEESE_V = 1.0;
	// Tube "radius" in noise units: bigger means wider tunnels.
	static constexpr double SPAGHETTI_R = 0.065;
	// Cheese carves where its field exceeds this; lower means bigger, more frequent caverns.
	static constexpr double CHEESE_T = 0.40;

	// Region mask: caves fade in as the mask rises from REGION_T to REGION_T + REGION_FADE.
	static constexpr double REGION_T = -0.15, REGION_FADE = 0.3;
	// Above this region strength, drop the cover so tunnels break the surface into mouths. Without
	// this the whole tube network stays sealed under the cover and never connects up to the surface.
	static constexpr double REGION_OPEN = 0.7;

	// Solid blocks of cover needed before carving. Thin tubes stay well under the surface so they
	// never scar the grass; only big caverns and ravines are allowed to break through into mouths.
	static constexpr int SUBMERGED_COVER = 6;          // seal the seabed under water
	// On flat land thin tubes stay buried so the grass isn't pocked; on mountains they open right
	// onto the cliff faces. Big caverns always reach the surface into wide natural mouths.
	static constexpr int SPAGHETTI_LAND_COVER = 4, MOUNTAIN_SPAGHETTI_COVER = 0, CHEESE_LAND_COVER = 0;
	static constexpr int FLOOR_Y = 5;                  // keep a solid floor at the bottom

	// Lava lakes. The world is tiled into warped cells, each pooling lava at its own flat level
	// between LAVA_MIN/MAX, so a connected pool only ever sits at one height. Where two cells of
	// different height meet, a thin band of rock (LAKE_WALL) stays solid below the higher level, so
	// their pools can't merge into one sloped body or leave an open void at the lava's edge.
	static constexpr int LAKE_CELL = 96;               // size of one flat lava region
	static constexpr int LAKE_WALL = 4;                // half-width of the rock band between regions
	static constexpr int LAKE_WARP = 28;               // wiggle the region edges off a straight grid
	static constexpr int LAKE_FILL = 128;              // out of 255, how many regions pool lava at all

	// How "mountainous" a column is, by surface height: drives cliff openings and guarantees caves
	// even where the region mask is low, so mountains are never left hollow-free.
	static constexpr int MOUNTAIN_START = Const::SEA_LEVEL + 24, MOUNTAIN_RANGE = 64;

	// Ravines: a center line, a sparse mask that keeps them rare, and their shape.
	static constexpr double RAVINE_HALF = 0.05, RAVINE_MASK_T = 0.6;
	static constexpr int RAVINE_FLOOR = 11, RAVINE_LAND_MARGIN = 2;

	// B reuses the same noise function as A, so it samples far away to stay independent.
	const dvec3 SPAGHETTI_OFFSET{ 1234.5, -555.5, 7777.5 };

	OctavePerlin m_spaghetti{ 1, 0.5, 1. / 56. };
	OctavePerlin m_cheese{ 2, 0.5, 1. / 64. };
	OctavePerlin m_region{ 2, 0.5, 1. / 300. };
	OctavePerlin m_ravinePath{ 2, 0.5, 1. / 96. };
	OctavePerlin m_ravineMask{ 2, 0.5, 1. / 400. };
	// Warps the lake cell grid so region edges curve instead of running along straight lines.
	OctavePerlin m_lakeWarp{ 2, 0.5, 1. / 220. };

	bool ravineCarved(const Column& col, int y) const;
	// Sets the column's lava level, and a wall level where it borders a region of different height.
	void lakeAt(ivec2 xz, Column& col) const;
	// Deterministic 0..255 hash of a cell plus a salt, for per-region levels without storing state.
	static int hash(int x, int z, int salt);
};
