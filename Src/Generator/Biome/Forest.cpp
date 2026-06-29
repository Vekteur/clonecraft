#include "Forest.h"

#include "World/WorldConstants.h"
#include "Util/Logger.h"

int Forest::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL + 8 + static_cast<int>(noise * 12);
}

Block Forest::getBlock(ivec3 pos, int depth) const {
	return layeredGround(pos, depth, BlockID::GRASS, BlockID::DIRT);
}

std::vector<StructureInfo> Forest::getStructures() const {
	return {
		{ StructureID::OAK, 0.3f },
		{ StructureID::BIG_OAK, 0.06f },
		{ StructureID::MAPLE, 0.05f },
		{ StructureID::CHERRY, 0.03f }
	};
}

double Forest::biomeValue(double temperature, double altitude) const {
	return medium(temperature) * high(altitude);
}