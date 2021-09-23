#include "Mountains.h"

#include "World/WorldConstants.h"
#include "Util/Logger.h"
#include "Maths/Converter.h"

int Mountains::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL + 40 + static_cast<int>(noise * 16);
}

Block Mountains::getBlock(ivec3 pos, int height) const {
	if (pos.y <= height - 1)
		return { BlockID::STONE };
	if (pos.y < Const::SEA_LEVEL)
		return { BlockID::WATER };

	return { BlockID::AIR };
}

std::vector<StructureInfo> Mountains::getStructures() const {
	return { { StructureID::TREE, 0.05f } };
}

double Mountains::biomeValue(double temperature, double altitude) const {
	return high(temperature) * medium(altitude);
}