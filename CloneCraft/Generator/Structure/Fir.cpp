#include "Fir.h"

#include "Generator/WorldGenerator.h"
#include "Maths/Dir3D.h"

Fir::Fir() : Structure({ 5, 7, 5 }) {
	ivec2 center = getCenterPos();
	add({ center.x, 6, center.y }, +BlockID::DARK_LEAVES);
	addSymetrically({ center.x + 1, 5, center.y }, +BlockID::DARK_LEAVES);
	addSymetrically({ center.x + 1, 3, center.y }, +BlockID::DARK_LEAVES);
	fillSymetrically({ center.x - 1, 2, center.y + 1 }, { center.x + 1, 2, center.y + 2 },
			+BlockID::DARK_LEAVES);
	fill({ center.x, 0, center.y }, { center.x, 5, center.y }, +BlockID::DARK_LOG);
}

bool Fir::isValidPos(ivec3 centerPos, BiomeID biomeID) const {
	const Biome& biome = g_worldGenerator.biomeMap().getBiome(biomeID);
	return centerPos.y >= Const::SEA_LEVEL &&
		biome.getBlock(centerPos + Dir3D::to_ivec3(Dir3D::DOWN), centerPos.y).id == +BlockID::SNOW;
}