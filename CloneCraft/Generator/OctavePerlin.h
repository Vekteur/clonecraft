#pragma once

#include "GlmCommon.h"
#include "PerlinNoise.h"

#include <vector>

class OctavePerlin
{
public:
	OctavePerlin(int octaves = 4, double persistence = 0.5, double frequency = 1.0);
	~OctavePerlin();

	double getNoise(vec2 pos);

private:
	std::vector<PerlinNoise> noises;

	int m_octaves; // The more octaves, the more accurate is the noise
	double m_persistence; // The less persistence, the less next octaves will have impact
	double m_frequency; // The more frequency, the more dense is the noise
};

