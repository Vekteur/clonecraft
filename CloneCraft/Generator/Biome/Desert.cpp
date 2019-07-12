#include "Desert.h"

#include "World/WorldConstants.h"
#include "Util/Logger.h"

int Desert::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL + 4 + static_cast<int>(noise * 8);
}

Block Desert::getBlock(int y, int height) const {
	if (y < height - 4)
		return { BlockID::STONE };
	if (y <= height - 1)
		return { BlockID::SAND };

	return { BlockID::AIR };
}

std::vector<StructureInfo> Desert::getStructures() const {
	return { };
}

double Desert::biomeValue(double temperature, double humidity) const {
	return high(temperature) * low(humidity);
}