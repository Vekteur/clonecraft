#pragma once

#include "Generator/CaveCarver.h"
#include "Generator/Noise/OctavePerlin.h"

#include <optional>

class BiomeMap;
class Chunk;
class ChunkGenerationInfo;
struct ColumnInfo;

// Turns a column's height and ruggedness into actual blocks. The surface sits where a density field
// crosses zero, and a 3D noise term lets it fold back into cliffs and overhangs. Caves are carved out
// afterwards.
class TerrainShaper {
public:
	explicit TerrainShaper(const BiomeMap& biomeMap);

	// Fills a chunk's blocks from its already computed heights and biomes.
	void fillBlocks(Chunk& chunk, const ChunkGenerationInfo& info) const;

	// First air block above the real terrain surface at a column, following the same distortion and
	// cave carving the block pass uses, so a structure sits exactly on the ground.
	std::optional<int> surfaceHeight(ivec2 xz, const ColumnInfo& col) const;

private:
	// 3D noise is sampled on a coarse lattice and interpolated, rather than per block, to stay cheap.
	// The vertical lattice is finer so overhangs aren't smoothed away.
	static constexpr int DENSITY_LATTICE_XZ = 4;
	static constexpr int DENSITY_LATTICE_Y = 2;
	// Vertical squash of the density noise. 1 is isotropic; below 1 stretches features vertically,
	// making overhangs large and clean. Above 1 multiplies folds but quickly looks terraced.
	static constexpr double DENSITY_VERTICAL_SCALE = 1.4;

	const BiomeMap& m_biomeMap;
	const CaveCarver m_caveCarver;
	// Distorts the surface into cliffs and overhangs, scaled per column by the biome's ruggedness.
	// The surface folds into an overhang where ruggedness exceeds about (wavelength / pi); the long
	// wavelength here makes big, clean cliffs rather than stacked terraces.
	OctavePerlin m_terrainNoise{ 2, 0.4, 1. / 72. };
};
