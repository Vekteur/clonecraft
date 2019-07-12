#include "Islands.h"

#include "World/WorldConstants.h"

int Islands::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL + 1 + static_cast<int>(noise * 32);
}

Block Islands::getBlock(int y, int height) const {
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

std::vector<StructureInfo> Islands::getStructures() const {
	return { { StructureID::TREE, 0.06f } };
}

bool Islands::isInBiome(double temperature, double humidity) const {
	return true;
}