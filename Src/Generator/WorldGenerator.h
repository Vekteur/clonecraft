#pragma once

#include "World/Chunk.h"
#include "Generator/Biome/BiomeMap.h"
#include "Generator/CaveCarver.h"
#include "Generator/Noise/OctavePerlin.h"

class WorldGenerator {
public:
	void loadChunk(Chunk& chunk) const;
	const BiomeMap& biomeMap() const;

private:
	// 3D noise is sampled on a coarse lattice and interpolated, rather than per block, to stay cheap.
	// The vertical lattice is finer so overhangs aren't smoothed away.
	static constexpr int DENSITY_LATTICE_XZ = 4;
	static constexpr int DENSITY_LATTICE_Y = 2;
	// Vertical squash of the density noise. 1 is isotropic; below 1 stretches features vertically,
	// making overhangs large and clean. Above 1 multiplies folds but quickly looks terraced.
	static constexpr double DENSITY_VERTICAL_SCALE = 1.4;

	void loadHeights(Chunk& chunk) const;
	void loadBlocks(Chunk& chunk) const;
	void loadStructures(Chunk& chunk) const;
	void loadStructure(Chunk& chunk, const Structure& structure, float freq, BiomeID biomeID) const;
	std::optional<ivec2> findLocalPos(const Structure& structure, ivec2 zonePos, float freq) const;
	void drawStructure(Chunk& chunk, const Structure& structure, ivec3 globalPos) const;

	const BiomeMap m_biomeMap;
	const CaveCarver m_caveCarver;
	// Distorts the surface into cliffs and overhangs, scaled per column by the biome's ruggedness.
	// The surface folds into an overhang where ruggedness exceeds about (wavelength / pi); the long
	// wavelength here makes big, clean cliffs rather than stacked terraces.
	OctavePerlin m_terrainNoise{ 2, 0.4, 1. / 72. };
};

extern WorldGenerator g_worldGenerator;