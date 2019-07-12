#include "Snow.h"

#include "World/WorldConstants.h"
#include "Util/Logger.h"

int Snow::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL + 8 + static_cast<int>(noise * 8);
}

Block Snow::getBlock(int y, int height) const {
	if (y < height - 4)
		return { BlockID::STONE };
	if (y < height - 3)
		return { BlockID::DIRT };
	if (y <= height - 1)
		return { BlockID::SNOW };
	if (y < Const::SEA_LEVEL - 1)
		return { BlockID::WATER };
	if (y < Const::SEA_LEVEL)
		return { BlockID::ICE };

	return { BlockID::AIR };
}

std::vector<StructureInfo> Snow::getStructures() const {
	return { { StructureID::TREE, 0.005f } };
}

double Snow::biomeValue(double temperature, double humidity) const {
	return low(temperature) * medium(humidity);
}