#include "LineBlockFinder.h"

#include <limits>
#include <algorithm>

#include "Converter.h"

LineBlockFinder::LineBlockFinder(vec3 pos, vec3 dir) : pos{ pos }, dir{ dir } 
{}

ivec3 LineBlockFinder::next() {
	ivec3 nextBlock;
	for (int axis = 0; axis < 3; ++axis) {
		if (posMod(std::abs(pos[axis]), 1.f) < epsilon) {
			nextBlock[axis] = static_cast<int>(std::round(pos[axis]));
			if (dir[axis] < 0)
				--nextBlock[axis];
		} else {
			nextBlock[axis] = static_cast<int>(floor(pos[axis]));
		}
	}

	float nextDistance;
	std::tie(pos, nextDistance) = nextIntersection();
	distance += nextDistance;

	return nextBlock;
}

float LineBlockFinder::getDistance() {
	return distance;
}

std::tuple<vec3, float> LineBlockFinder::nextIntersection() {
	float minTime = std::numeric_limits<float>::infinity();
	for (int axis = 0; axis < 3; ++axis) {
		if (std::abs(dir[axis]) < epsilon)
			continue;
		float inter = (dir[axis] > 0.f) ? floor(pos[axis] + 1) : ceil(pos[axis] - 1);
		float time = std::abs((inter - pos[axis]) / dir[axis]);
		minTime = std::min(minTime, time);
	}

	vec3 inter = pos + dir * minTime;
	float dist = glm::distance(pos, inter);
	return { inter, dist };
}
