#include "Swamp.h"

#include "World/WorldConstants.h"

int Swamp::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL + 4 + static_cast<int>(noise * 32);
}

Block Swamp::getBlock(ivec3 pos, int depth) const {
	return layeredGround(pos, depth, BlockID::GRASS, BlockID::DIRT);
}

std::vector<StructureInfo> Swamp::getStructures() const {
	return { { StructureID::TREE, 0.1f } };
}

double Swamp::biomeValue(double temperature, double altitude) const {
	return low(temperature) * medium(altitude);
}