#include "Forest.h"

#include "World/WorldConstants.h"
#include "Util/Logger.h"

int Forest::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL + 6 + static_cast<int>(noise * 12);
}

Block Forest::getBlock(ivec3 pos, int height) const {
	if (pos.y < height - 4)
		return { BlockID::STONE };
	if (pos.y < Const::SEA_LEVEL + 1 && height - 3 <= pos.y && pos.y <= height - 1)
		return { BlockID::SAND };
	if (pos.y < height - 1)
		return { BlockID::DIRT };
	if (pos.y == height - 1)
		return { BlockID::GRASS };
	if (pos.y < Const::SEA_LEVEL)
		return { BlockID::WATER };

	return { BlockID::AIR };
}

std::vector<StructureInfo> Forest::getStructures() const {
	return { { StructureID::TREE, 0.3f } };
}

double Forest::biomeValue(double temperature, double altitude) const {
	return medium(temperature) * high(altitude);
}