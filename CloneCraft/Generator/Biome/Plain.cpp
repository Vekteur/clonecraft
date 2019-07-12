#include "Plain.h"

#include "World/WorldConstants.h"
#include "Util/Logger.h"

int Plain::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL + 4 + static_cast<int>(noise * 8);
}

Block Plain::getBlock(int y, int height) const {
	if (y < height - 4)
		return { BlockID::STONE };
	if (y < Const::SEA_LEVEL + 1 && height - 3 <= y && y <= height - 1)
		return { BlockID::SAND };
	if (y < height - 1)
		return { BlockID::DIRT };
	if (y == height - 1)
		return { BlockID::GRASS };
	if (y < Const::SEA_LEVEL)
		return { BlockID::WATER };

	return { BlockID::AIR };
}

std::vector<StructureInfo> Plain::getStructures() const {
	return { { StructureID::TREE, 0.005f } };
}

double Plain::biomeValue(double temperature, double humidity) const {
	return high(temperature) * medium(humidity);
}