#include "Ocean.h"

#include "World/WorldConstants.h"
#include "Util/Logger.h"

int Ocean::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL - 40 + static_cast<int>(noise * 20);
}

Block Ocean::getBlock(ivec3 pos, int depth) const {
	return layeredGround(pos, depth, BlockID::SAND, BlockID::SAND);
}

std::vector<StructureInfo> Ocean::getStructures() const {
	return { };
}

double Ocean::biomeValue(double temperature, double altitude) const {
	return low(temperature) * low(altitude);
}