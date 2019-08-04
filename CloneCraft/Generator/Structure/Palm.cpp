#include "Palm.h"

#include "Generator/WorldGenerator.h"
#include "Maths/Dir3D.h"

Palm::Palm() : Structure({ 9, 7, 9 }) {
	ivec2 center = getCenterPos();
	add({ center.x, 6, center.y }, +BlockID::LEAVES);
	addSymetrically({ center.x + 1, 5, center.y }, +BlockID::LEAVES);
	fillSymetrically({ center.x + 1, 6, center.y }, { center.x + 2, 6, center.y },
		+BlockID::LEAVES);
	fillSymetrically({ center.x + 3, 5, center.y }, { center.x + 4, 5, center.y },
		+BlockID::LEAVES);
	fill({ center.x, 0, center.y + 1 }, { center.x, 3, center.y + 1 }, +BlockID::LOG);
	fill({ center.x, 3, center.y }, { center.x, 5, center.y }, +BlockID::LOG);
}

bool Palm::isValidPos(ivec3 centerPos, BiomeID biomeID) const {
	const Biome& biome = g_worldGenerator.biomeMap().getBiome(biomeID);
	return centerPos.y >= Const::SEA_LEVEL &&
		biome.getBlock(centerPos + Dir3D::to_ivec3(Dir3D::DOWN), centerPos.y).id == +BlockID::SAND;
}