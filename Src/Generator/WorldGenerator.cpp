#include "WorldGenerator.h"

WorldGenerator::WorldGenerator() {}

void WorldGenerator::loadChunk(Chunk& chunk) const {
	loadHeights(chunk);
	m_terrain.fillBlocks(chunk, chunk.chunkInfo());
	m_structures.place(chunk);
}

const BiomeMap& WorldGenerator::biomeMap() const {
	return m_biomeMap;
}

void WorldGenerator::loadHeights(Chunk& chunk) const {
	for (int x = 0; x < Const::SECTION_SIDE; ++x)
		for (int z = 0; z < Const::SECTION_SIDE; ++z) {
			ivec2 innerPos{ x, z };
			ivec2 pos = innerPos + chunk.getPosition() * Const::SECTION_SIDE;
			ColumnInfo col = m_biomeMap.getColumn(pos);
			chunk.chunkInfo().biome(innerPos) = col.biome;
			chunk.chunkInfo().height(innerPos) = col.height;
			chunk.chunkInfo().ruggedness(innerPos) = col.ruggedness;
		}
}

void WorldGenerator::unloadFarStructures(ivec2 centerChunk, int viewDistanceChunks) const {
	m_structures.unloadFar(centerChunk, viewDistanceChunks);
}

WorldGenerator g_worldGenerator;
