#include "Biome.h"

bool Biome::low(double value) const {
	return value < -threshold;
}

bool Biome::medium(double value) const
{
	return -threshold <= value && value <= threshold;
}

bool Biome::high(double value) const {
	return threshold < value;
}
