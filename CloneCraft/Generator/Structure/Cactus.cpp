#include "Cactus.h"

#include "Generator/WorldGenerator.h"

Cactus::Cactus() : Structure({ 1, 5, 1 }) {
	ivec2 center = getCenterPos();
	fill({ center.x, 0, center.y }, { center.x, 3, center.y }, +BlockID::CACTUS);
}

bool Cactus::isValidPos(ivec2 pos) const {
	ivec2 centerPos = getCenterPos(pos);
	const Biome& biome = g_worldGenerator.biomeMap().getBiome(centerPos);
	int height = g_worldGenerator.biomeMap().getHeight(centerPos);
	return biome.getBlock({ centerPos.x, height - 1, centerPos.y }, height).id == +BlockID::SAND;
}
