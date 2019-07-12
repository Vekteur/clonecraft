#include "ExtremeMountains.h"

#include "World/WorldConstants.h"
#include "Util/Logger.h"

int ExtremeMountains::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL + 60 + static_cast<int>(noise * 32);
}

Block ExtremeMountains::getBlock(int y, int height) const {
	if (y <= height - 1)
		return { BlockID::STONE };
	if (y < Const::SEA_LEVEL)
		return { BlockID::WATER };

	return { BlockID::AIR };
}

std::vector<StructureInfo> ExtremeMountains::getStructures() const {
	return { { StructureID::TREE, 0.005f } };
}

double ExtremeMountains::biomeValue(double temperature, double humidity) const {
	return low(temperature) * low(humidity);
}