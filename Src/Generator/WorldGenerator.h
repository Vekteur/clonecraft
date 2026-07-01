#pragma once

#include "World/Chunk.h"
#include "Generator/Biome/BiomeMap.h"
#include "Generator/TerrainShaper.h"
#include "Generator/Structure/StructureGenerator.h"

class WorldGenerator {
public:
	WorldGenerator();

	void loadChunk(Chunk& chunk) const;
	const BiomeMap& biomeMap() const;

	// Drops cached structure cells far from the player. Called by the orchestrator thread, like the
	// chunk unloading. viewDistanceChunks is the chunk view radius (margin is added on top).
	void unloadFarStructures(ivec2 centerChunk, int viewDistanceChunks) const;

private:
	void loadHeights(Chunk& chunk) const;

	const BiomeMap m_biomeMap;
	const TerrainShaper m_terrain{ m_biomeMap };
	const StructureGenerator m_structures{ m_biomeMap, m_terrain };
};

extern WorldGenerator g_worldGenerator;
