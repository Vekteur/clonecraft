#include "Desert.h"

#include "World/WorldConstants.h"
#include "Util/Logger.h"

int Desert::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL + 6 + static_cast<int>(noise * 8);
}

Block Desert::getBlock(ivec3 pos, int height) const {
	if (pos.y < height - 4)
		return { BlockID::STONE };
	if (pos.y <= height - 1)
		return { BlockID::SAND };
	if (pos.y < Const::SEA_LEVEL)
		return { BlockID::WATER };

	return { BlockID::AIR };
}

std::vector<StructureInfo> Desert::getStructures() const {
	return { { StructureID::DEAD_TREE, 0.001f } /*, { StructureID::CACTUS, 0.005f }*/ };
}

double Desert::biomeValue(double temperature, double altitude) const {
	return high(temperature) * low(altitude);
}