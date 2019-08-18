#pragma once

#include "Maths/GlmCommon.h"

#include <array>

class PerlinNoise {
public:
	static double getNoise(dvec2 pos);
	static double getNoise(dvec2 pos, double frequency);
	static std::array<int, 512> perm;

private:
	static double lerp(double amount, double left, double right);
	static double fade(double t);
	static double grad(int hash, double x, double y);
};

