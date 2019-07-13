#include "Snow.h"

#include "World/WorldConstants.h"
#include "Util/Logger.h"

int Snow::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL + 8 + static_cast<int>(noise * 8);
}

Block Snow::getBlock(ivec3 pos, int height) const {
	if (pos.y < height - 4)
		return { BlockID::STONE };
	if (pos.y < height - 3)
		return { BlockID::DIRT };
	if (pos.y <= height - 1)
		return { BlockID::SNOW };
	if (pos.y < Const::SEA_LEVEL - 1)
		return { BlockID::WATER };
	if (pos.y < Const::SEA_LEVEL)
		return { BlockID::ICE };

	return { BlockID::AIR };
}

std::vector<StructureInfo> Snow::getStructures() const {
	return { { StructureID::FIR, 0.03f } };
}

double Snow::biomeValue(double temperature, double altitude) const {
	return low(temperature) * high(altitude);
}