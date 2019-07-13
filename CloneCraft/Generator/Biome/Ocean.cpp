#include "Ocean.h"

#include "World/WorldConstants.h"
#include "Util/Logger.h"

int Ocean::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL - 40 + static_cast<int>(noise * 8);
}

Block Ocean::getBlock(ivec3 pos, int height) const {
	if (pos.y < height - 3)
		return { BlockID::STONE };
	if (pos.y < height)
		return { BlockID::SAND };
	if (pos.y < Const::SEA_LEVEL)
		return { BlockID::WATER };

	return { BlockID::AIR };
}

std::vector<StructureInfo> Ocean::getStructures() const {
	return { };
}

double Ocean::biomeValue(double temperature, double altitude) const {
	return low(temperature) * low(altitude);
}