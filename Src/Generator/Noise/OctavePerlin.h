#pragma once

#include "Maths/GlmCommon.h"

#include <vector>

class OctavePerlin {
public:
	OctavePerlin(int octaves = 4, double persistence = 0.5, double frequency = 1.0);

	double getNoise(dvec2 pos) const;
	double getNoise(dvec3 pos) const;
	// Ridged noise: sharp crests instead of rounded hills, in [0, 1]. Great for mountain ridges.
	double getRidgedNoise(dvec2 pos) const;

private:
	int m_octaves; // The more octaves, the more accurate is the noise
	double m_persistence; // The less persistence, the less next octaves will have impact
	double m_frequency; // The more frequency, the more dense is the noise
};

