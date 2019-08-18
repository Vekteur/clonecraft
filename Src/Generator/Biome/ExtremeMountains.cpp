#include "ExtremeMountains.h"

#include "Generator/Noise/PerlinNoise.h"
#include "World/WorldConstants.h"
#include "Maths/Converter.h"
#include "Util/Logger.h"

int ExtremeMountains::getHeight(ivec2 pos) const {
	double noise = perlin.getNoise(static_cast<dvec2>(pos));
	return Const::SEA_LEVEL + 100 + static_cast<int>(noise * 64);
}

Block ExtremeMountains::getBlock(ivec3 pos, int height) const {
	// Mean snow level in y = 162 with noise
	if (height - 4 <= pos.y && pos.y <= height - 1 &&
			162 + snowPerlin.getNoise(Converter::to2D(pos)) * 30 < height)
		return { BlockID::SNOW };
	if (pos.y < height - 1)
		return { BlockID::STONE };
	if (pos.y < Const::SEA_LEVEL)
		return { BlockID::WATER };

	return { BlockID::AIR };
}

std::vector<StructureInfo> ExtremeMountains::getStructures() const {
	return { { StructureID::TREE, 0.005f } };
}

double ExtremeMountains::biomeValue(double temperature, double altitude) const {
	return high(temperature) * high(altitude);
}