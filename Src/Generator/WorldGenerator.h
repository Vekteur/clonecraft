#pragma once

#include "World/Chunk.h"
#include "Generator/Biome/BiomeMap.h"

class WorldGenerator {
public:
	void loadChunk(Chunk& chunk) const;
	const BiomeMap& biomeMap() const;

private:
	void loadHeights(Chunk& chunk) const;
	void loadBlocks(Chunk& chunk) const;
	void loadStructures(Chunk& chunk) const;
	void loadStructure(Chunk& chunk, const Structure& structure, float freq, BiomeID biomeID) const;
	std::optional<ivec2> findLocalPos(const Structure& structure, ivec2 zonePos, float freq) const;
	void drawStructure(Chunk& chunk, const Structure& structure, ivec3 globalPos) const;

	const BiomeMap m_biomeMap;
};

extern WorldGenerator g_worldGenerator;