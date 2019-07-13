#include "Tree.h"

#include "Generator/WorldGenerator.h"

Tree::Tree() : Structure({ 5, 7, 5 }) {
	ivec2 center = getCenterPos();
	fill({ center.x - 2, 3, center.y - 2 }, { center.x + 2, 4, center.y + 2 }, +BlockID::LEAVES);
	fill({ center.x - 1, 5, center.y - 1 }, { center.x + 1, 6, center.y + 1 }, +BlockID::LEAVES);
	fill({ center.x, 0, center.y }, { center.x, 4, center.y }, +BlockID::LOG);
}

bool Tree::isValidPos(ivec2 pos) const {
	ivec2 centerPos = getCenterPos(pos);
	const Biome& biome = g_worldGenerator.biomeMap().getBiome(centerPos);
	int height = g_worldGenerator.biomeMap().getHeight(centerPos);
	return biome.getBlock({ centerPos.x, height - 1, centerPos.y }, height).id == +BlockID::GRASS;
}
