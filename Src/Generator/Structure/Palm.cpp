#include "Palm.h"

#include "Generator/WorldGenerator.h"
#include "Maths/Dir3D.h"
#include "Maths/Dir2D.h"

Palm::Palm() : Structure({ 9, 7, 9 }) {
	ivec2 center = Structure::getCenterPos();
	add({ center.x, 6, center.y }, +BlockID::LEAVES);
	addSymetrically({ center.x + 1, 5, center.y }, +BlockID::LEAVES);
	fillSymetrically({ center.x + 1, 6, center.y }, { center.x + 2, 6, center.y },
		+BlockID::LEAVES);
	fillSymetrically({ center.x + 3, 5, center.y }, { center.x + 4, 5, center.y },
		+BlockID::LEAVES);
	fill({ center.x, 0, center.y + 1 }, { center.x, 3, center.y + 1 }, +BlockID::LOG);
	fill({ center.x, 3, center.y }, { center.x, 5, center.y }, +BlockID::LOG);
}

ivec2 Palm::getSupportPos(ivec2 globalPos) const {
	return getCenterPos(globalPos) + Dir2D::to_ivec2(Dir2D::RIGHT);
}

bool Palm::isValidPos(ivec3 centerPos, BiomeID biomeID) const {
	const Biome& biome = g_worldGenerator.biomeMap().getBiome(biomeID);
	return centerPos.y >= Const::SEA_LEVEL &&
		biome.getBlock(centerPos + Dir3D::to_ivec3(Dir3D::DOWN), centerPos.y).id == +BlockID::SAND;
}