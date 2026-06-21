#include "Biome.h"

#include "World/WorldConstants.h"

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
	return step(value, -threshold);
}

double Biome::medium(double value) {
	return step(-value, threshold) * step(value, threshold);
}

double Biome::high(double value) {
	return step(-value, -threshold);
}

double Biome::step(double value, double threshold) {
	double diff = threshold + transition - value;
	if (diff > transition) return 1.;
	if (diff > 0) return smooth(diff / transition);
	return 0.;
}

double Biome::smooth(double x) {
	return x * x * x * (x * (x * 6 - 15) + 10);
}