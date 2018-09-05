#pragma once

#include "GlmCommon.h"
#include "PerlinNoise.h"

#include <vector>

class OctavePerlin
{
public:
	OctavePerlin(int octaves = 4, float persistence = 0.5, float frequency = 1.0);
	~OctavePerlin();

	double getNoise(vec2 pos);

private:
	std::vector<PerlinNoise> noises;

	int m_octaves; // The more octaves, the more accurate is the noise
	float m_persistence; // The less persistence, the less next octaves will have impact
	float m_frequency; // The more frequency, the more dense is the noise
};

