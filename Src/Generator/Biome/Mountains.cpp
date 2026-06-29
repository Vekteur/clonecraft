#include "Mountains.h"

#include "World/WorldConstants.h"
#include "Util/Logger.h"
#include "Maths/Converter.h"

int Mountains::getHeight(ivec2 pos) const {
	dvec2 p = static_cast<dvec2>(pos);
	// Warp the lookup so ridges meander instead of looking grid-aligned
	dvec2 offset{ warp.getNoise(p), warp.getNoise(p + 137.5) };
	double ridge = perlin.getRidgedNoise(p + offset * 60.);
	return Const::SEA_LEVEL + 24 + static_cast<int>(ridge * 72);
}

Block Mountains::getBlock(ivec3 pos, int depth) const {
	// Green lower slopes, bare rock higher up
	if (pos.y < Const::SEA_LEVEL + 48)
		return layeredGround(pos, depth, BlockID::GRASS, BlockID::DIRT);
	return { BlockID::STONE };
}

std::vector<StructureInfo> Mountains::getStructures() const {
	return { { StructureID::OAK, 0.05f } };
}

double Mountains::biomeValue(double temperature, double altitude) const {
	return high(temperature) * medium(altitude);
}