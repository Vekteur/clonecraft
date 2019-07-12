#include "Ocean.h"

#include "World/WorldConstants.h"
#include "Util/Logger.h"

int Ocean::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL - 40 + static_cast<int>(noise * 8);
}

Block Ocean::getBlock(int y, int height) const {
	if (y < height - 4)
		return { BlockID::STONE };
	if (height - 3 <= y && y <= height - 1)
		return { BlockID::SAND };
	if (y < Const::SEA_LEVEL)
		return { BlockID::WATER };

	return { BlockID::AIR };
}

std::vector<StructureInfo> Ocean::getStructures() const {
	return { };
}

double Ocean::biomeValue(double temperature, double humidity) const {
	return low(temperature) * high(humidity);
}