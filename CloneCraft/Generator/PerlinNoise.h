#include <GlmCommon.h>
#include <array>

#pragma once
class PerlinNoise
{
public:
	PerlinNoise();
	~PerlinNoise();

	double getNoise(vec2 pos);

private:
	static double lerp(double amount, double left, double right);
	static double fade(double t);
	static double grad(int hash, double x, double y);

	static std::array<int, 512> perm;
};

