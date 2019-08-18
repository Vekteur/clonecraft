#include "Biome.h"

double Biome::threshold = 0.14, Biome::transition = 0.15;

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