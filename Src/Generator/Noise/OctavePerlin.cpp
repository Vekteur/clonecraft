#include "OctavePerlin.h"

#include "PerlinNoise.h"

#include <cmath>

OctavePerlin::OctavePerlin(int octaves, double persistence, double frequency)
	: m_octaves{ octaves }, m_persistence{ persistence }, m_frequency{ frequency } { }

double OctavePerlin::getNoise(dvec2 pos) const {
	double total = 0;
	double frequency = m_frequency;
	double amplitude = 1;
	double maxTotal = 0;
	for (int i = 0; i < m_octaves; i++) {
		total += PerlinNoise::getNoise(pos, frequency) * amplitude;
		maxTotal += amplitude;
		amplitude *= m_persistence;
		frequency *= 2.;
	}

	return total / maxTotal;
}

double OctavePerlin::getNoise(dvec3 pos) const {
	double total = 0;
	double frequency = m_frequency;
	double amplitude = 1;
	double maxTotal = 0;
	for (int i = 0; i < m_octaves; i++) {
		total += PerlinNoise::getNoise(pos, frequency) * amplitude;
		maxTotal += amplitude;
		amplitude *= m_persistence;
		frequency *= 2.;
	}

	return total / maxTotal;
}

double OctavePerlin::getRidgedNoise(dvec2 pos) const {
	double total = 0;
	double frequency = m_frequency;
	double amplitude = 1;
	double maxTotal = 0;
	for (int i = 0; i < m_octaves; i++) {
		// Fold the noise around zero to make a sharp crest, then square to sharpen it further
		double ridge = 1. - std::abs(PerlinNoise::getNoise(pos, frequency));
		total += ridge * ridge * amplitude;
		maxTotal += amplitude;
		amplitude *= m_persistence;
		frequency *= 2.;
	}

	return total / maxTotal;
}