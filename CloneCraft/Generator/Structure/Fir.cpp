#include "Fir.h"

#include "Generator/WorldGenerator.h"

Fir::Fir() : Structure({ 5, 7, 5 }) {
	ivec2 center = getCenterPos();
	add({ center.x, 6, center.y }, +BlockID::DARK_LEAVES);
	addSymetrically({ center.x + 1, 5, center.y }, +BlockID::DARK_LEAVES);
	addSymetrically({ center.x + 1, 3, center.y }, +BlockID::DARK_LEAVES);
	fillSymetrically({ center.x - 1, 2, center.y + 1 }, { center.x + 1, 2, center.y + 2 },
			+BlockID::DARK_LEAVES);
	fill({ center.x, 0, center.y }, { center.x, 5, center.y }, +BlockID::DARK_LOG);
}

bool Fir::isValidPos(ivec2 pos) const {
	ivec2 centerPos = getCenterPos(pos);
	const Biome& biome = g_worldGenerator.biomeMap().getBiome(centerPos);
	int height = g_worldGenerator.biomeMap().getHeight(centerPos);
	return height >= Const::SEA_LEVEL &&
			biome.getBlock({ centerPos.x, height - 1, centerPos.y }, height).id == +BlockID::SNOW;
}
