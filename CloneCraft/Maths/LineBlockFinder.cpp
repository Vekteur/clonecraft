#include "LineBlockFinder.h"

#include <limits>
#include <algorithm>

#include "Converter.h"
#include "Util/Logger.h"

LineBlockFinder::LineBlockFinder(vec3 pos, vec3 dir) : pos{ pos }, dir{ dir } 
{}

ivec3 LineBlockFinder::next() {
	ivec3 nextBlock;
	for (int axe = 0; axe < 3; ++axe) {
		if (posMod(pos[axe], 1.f) < epsilon) {
			nextBlock[axe] = static_cast<int>((dir[axe] >= 0) ? pos[axe] : pos[axe] - 1.f);
		} else {
			nextBlock[axe] = static_cast<int>(floor(pos[axe]));
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
	float minRatio = std::numeric_limits<float>::infinity();
	for (int currAxe = 0; currAxe < 3; ++currAxe) {
		if (abs(dir[currAxe]) < epsilon)
			continue;
		float inter = (dir[currAxe] > 0.f) ? floor(pos[currAxe] + 1) : ceil(pos[currAxe] - 1);
		float ratio = abs((inter - pos[currAxe]) / dir[currAxe]);
		minRatio = std::min(minRatio, ratio);
	}

	vec3 inter;
	for (int axe = 0; axe < 3; ++axe) {
		inter[axe] = pos[axe] + dir[axe] * minRatio;
	}
	float dist = glm::distance(pos, inter);
	return { inter, dist };
}
