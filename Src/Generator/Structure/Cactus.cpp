#include "Cactus.h"

#include "Generator/WorldGenerator.h"

Cactus::Cactus() : Structure({ 1, 5, 1 }) {
	ivec2 center = getCenterPos();
	fill({ center.x, 0, center.y }, { center.x, 3, center.y }, +BlockID::CACTUS);
}

bool Cactus::isValidPos(ivec3 centerPos, BiomeID biomeID) const {
	const Biome& biome = g_worldGenerator.biomeMap().getBiome(biomeID);
	return centerPos.y >= Const::SEA_LEVEL &&
		biome.getBlock(centerPos + Dir3D::to_ivec3(Dir3D::DOWN), centerPos.y).id == +BlockID::SAND;
}