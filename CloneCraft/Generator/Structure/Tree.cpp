#include "Tree.h"

#include "Generator/WorldGenerator.h"

Tree::Tree() : Structure({ 5, 7, 5 }) {
	ivec2 center = getCenterPos();
	fill({ center.x - 2, 3, center.y - 2 }, { center.x + 2, 4, center.y + 2 }, +BlockID::LEAVES);
	fill({ center.x - 1, 5, center.y - 1 }, { center.x + 1, 6, center.y + 1 }, +BlockID::LEAVES);
	fill({ center.x, 0, center.y }, { center.x, 4, center.y }, +BlockID::LOG);
}

bool Tree::isValidPos(ivec3 centerPos, BiomeID biomeID) const {
	const Biome& biome = g_worldGenerator.biomeMap().getBiome(biomeID);
	return centerPos.y >= Const::SEA_LEVEL &&
		biome.getBlock(centerPos + Dir3D::to_ivec3(Dir3D::DOWN), centerPos.y).id == +BlockID::GRASS;
}