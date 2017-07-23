#pragma once

#include "GlmCommon.h"
#include "PerlinNoise.h"

#include <vector>

class OctavePerlin
{
public:
	OctavePerlin(int octaves = 4, double persistence = 1.0, double frequency = 1.0);
	~OctavePerlin();

	double getNoise(vec2 pos);

private:
	std::vector<PerlinNoise> noises;

	int m_octaves;
	double m_persistence;
	double m_frequency;
};

