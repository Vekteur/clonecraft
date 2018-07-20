#include "OctavePerlin.h"

OctavePerlin::OctavePerlin(int octaves, float persistence, float frequency)
	: m_octaves{ octaves }, m_persistence{ persistence }, m_frequency{ frequency } {
	for (int i = 0; i < m_octaves; ++i) {
		noises.push_back(PerlinNoise());
	}
}

OctavePerlin::~OctavePerlin() {
}

double OctavePerlin::getNoise(vec2 pos) {
	float total = 0;
	float frequency = m_frequency;
	float amplitude = 1;
	float maxTotal = 0;
	for (int i = 0; i < noises.size(); i++) {
		total += noises[i].getNoise(pos * frequency) * amplitude;

		maxTotal += amplitude;

		amplitude *= m_persistence;
		frequency *= 2;
	}

	return total / maxTotal;
}