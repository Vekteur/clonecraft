#include "ExtremeMountains.h"

#include "Generator/Noise/PerlinNoise.h"
#include "World/WorldConstants.h"
#include "Maths/Converter.h"
#include "Util/Logger.h"

int ExtremeMountains::getHeight(ivec2 pos) const {
	dvec2 p = static_cast<dvec2>(pos);
	// Warp the lookup so ridges meander instead of looking grid-aligned
	dvec2 offset{ warp.getNoise(p), warp.getNoise(p + 137.5) };
	double ridge = perlin.getRidgedNoise(p + offset * 80.);
	return Const::SEA_LEVEL + 48 + static_cast<int>(ridge * 200);
}

Block ExtremeMountains::getBlock(ivec3 pos, int depth) const {
	// Snow caps the surface above a noisy snow line, rock everywhere below
	double snowLine = 170 + snowPerlin.getNoise(Converter::to2D(pos)) * 30;
	if (pos.y > snowLine && depth <= 3)
		return { BlockID::SNOW };
	return { BlockID::STONE };
}

std::vector<StructureInfo> ExtremeMountains::getStructures() const {
	return { { StructureID::OAK, 0.005f } };
}

double ExtremeMountains::biomeValue(double temperature, double altitude) const {
	return high(temperature) * high(altitude);
}