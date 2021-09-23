#pragma once

#include <tuple>

#include <Maths/GlmCommon.h>

class LineBlockFinder {
public:
	LineBlockFinder(vec3 pos, vec3 dir);

	ivec3 next();
	float getDistance();

private:
	const float epsilon = 1e-6f;
	vec3 pos;
	vec3 dir;
	float distance = 0.f;

	std::tuple<vec3, float> nextIntersection();
};

