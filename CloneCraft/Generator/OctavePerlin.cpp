#include "OctavePerlin.h"

OctavePerlin::OctavePerlin(int octaves, double persistence, double frequency)
	: m_octaves{ octaves }, m_persistence{ persistence }, m_frequency{ frequency } {
	for (int i = 0; i < m_octaves; i++)
		noises.push_back(PerlinNoise());
}

OctavePerlin::~OctavePerlin() {
}

double OctavePerlin::getNoise(vec2 pos) {
	double total = 0;
	double frequency = m_frequency;
	double amplitude = 1;
	double maxTotal = 0;
	for (int i = 0; i < noises.size(); i++) {
		total += noises[i].getNoise(vec2{ pos.x * frequency, pos.y * frequency }) * amplitude;

		maxTotal += amplitude;

		amplitude *= m_persistence;
		frequency *= 2;
	}

	return total / maxTotal;
}