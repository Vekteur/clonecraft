#include <GlmCommon.h>

#pragma once
class PerlinNoise
{
public:
	PerlinNoise();
	~PerlinNoise();

	static void init();

	double getNoise(vec2 pos);

private:
	static double lerp(double amount, double left, double right);
	static double fade(double t);
	static double grad(int hash, double x, double y);

	static int perm[512];
};

