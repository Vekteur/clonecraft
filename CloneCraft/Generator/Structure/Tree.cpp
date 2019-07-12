#include "Tree.h"

#include "World/ChunkMap.h"
#include "Util/Logger.h"
#include "Generator/Noise/PerlinNoise.h"
#include "Maths/Converter.h"
#include "Generator/WorldGenerator.h"

#include <random>

const ivec3 Tree::s_size{ s_sizeX, s_sizeY, s_sizeZ };

Tree::Tree() : Structure() {
	ivec2 center = getCenterPos({ 0, 0 });
	fill({ center.x - 2, 3, center.y - 2 }, { center.x + 2, 4, center.y + 2 }, +BlockID::LEAVES);
	fill({ center.x - 1, 5, center.y - 1 }, { center.x + 1, 6, center.y + 1 }, +BlockID::LEAVES);
	fill({ center.x, 0, center.y }, { center.x, 4, center.y }, +BlockID::LOG);
}

bool Tree::isValidPos(ivec2 pos) const {
	ivec2 centerPos = getCenterPos(pos);
	const Biome& biome = g_worldGenerator.biomeMap().getBiome(centerPos);
	int height = g_worldGenerator.biomeMap().getHeight(pos);
	return biome.getBlock(height - 1, height).id == +BlockID::GRASS;
}

Block Tree::getBlock(ivec3 pos) const {
	return m_blocks.at(pos);
}

ivec3 Tree::size() const { 
	return s_size; 
}

void Tree::fill(ivec3 low, ivec3 high, Block block) {
	for (int y = low.y; y <= high.y; ++y)
		for (int x = low.x; x <= high.x; ++x)
			for (int z = low.z; z <= high.z; ++z) {
				m_blocks.at({ x, y, z }) = block;
			}
}
