#include "Biome.h"

#include "World/WorldConstants.h"
#include "Maths/MiscMath.h"

double Biome::threshold = 0.14, Biome::transition = 0.15;

Block Biome::layeredGround(ivec3 pos, int depth, BlockID surface, BlockID subsurface) {
	if (pos.y <= Const::SEA_LEVEL && depth <= 2)
		return { BlockID::SAND };
	if (depth == 0)
		return { surface };
	if (depth <= 3)
		return { subsurface };
	return { BlockID::STONE };
}

double Biome::low(double value) {
	// 1 -> 0 over [-threshold, -threshold + transition]
	return step(value, -threshold);
}

double Biome::medium(double value) {
	// 0 -> 1 over [-threshold - transition, -threshold] then 1 -> 0 over [threshold, threshold + transition]
	return step(-value, threshold) * step(value, threshold);
}

double Biome::high(double value) {
	// 0 -> 1 over [threshold - transition, threshold]
	return step(-value, -threshold);
}

double Biome::step(double value, double threshold) {
	double diff = threshold + transition - value;
	if (diff > transition) return 1.;
	// threshold <= value < threshold + transition
	if (diff > 0) return math::smoothstep(diff / transition);
	return 0.;
}
