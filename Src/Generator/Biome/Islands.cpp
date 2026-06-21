#include "Islands.h"

#include "World/WorldConstants.h"

int Islands::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL - 1 + static_cast<int>(noise * 32);
}

Block Islands::getBlock(ivec3 pos, int depth) const {
	return layeredGround(pos, depth, BlockID::GRASS, BlockID::DIRT);
}

std::vector<StructureInfo> Islands::getStructures() const {
	return { { StructureID::PALM, 0.06f } };
}

double Islands::biomeValue(double temperature, double altitude) const {
	return medium(temperature) * low(altitude);
}