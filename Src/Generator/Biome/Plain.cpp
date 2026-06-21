#include "Plain.h"

#include "World/WorldConstants.h"
#include "Util/Logger.h"

int Plain::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL + 4 + static_cast<int>(noise * 4);
}

Block Plain::getBlock(ivec3 pos, int depth) const {
	return layeredGround(pos, depth, BlockID::GRASS, BlockID::DIRT);
}

std::vector<StructureInfo> Plain::getStructures() const {
	return { { StructureID::TREE, 0.001f } };
}

double Plain::biomeValue(double temperature, double altitude) const {
	return medium(temperature) * medium(altitude);
}