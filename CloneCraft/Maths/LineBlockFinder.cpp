#include "LineBlockFinder.h"

#include <limits>

#include "Converter.h"
#include "Logger.h"

LineBlockFinder::LineBlockFinder(vec3 pos, vec3 dir) : pos{ pos }, dir{ dir } 
{}

ivec3 LineBlockFinder::next() {
	ivec3 nextBlock;
	for (int axe = 0; axe < 3; ++axe) {
		if (posMod(pos[axe], 1.f) < epsilon) {
			nextBlock[axe] = (dir[axe] >= 0) ? pos[axe] : pos[axe] - 1;
		} else {
			nextBlock[axe] = floor(pos[axe]);
		}
	}

	auto nextInter = nextIntersection();
	pos = std::get<0>(nextInter);
	distance += std::get<1>(nextInter);

	return nextBlock;
}

float LineBlockFinder::getDistance() {
	return distance;
}

std::tuple<vec3, float> LineBlockFinder::nextIntersection() {
	float minDist = std::numeric_limits<float>::infinity();
	vec3 nearestInter;
	for (int currAxe = 0; currAxe < 3; ++currAxe) {
		vec3 inter;
		if (abs(dir[currAxe]) < epsilon)
			continue;
		inter[currAxe] = (dir[currAxe] > 0.f) ? floor(pos[currAxe] + 1) : ceil(pos[currAxe] - 1);
		float ratio = abs((inter[currAxe] - pos[currAxe]) / dir[currAxe]);
		for (int axe = 0; axe < 3; ++axe) {
			if (axe != currAxe)
				inter[axe] = pos[axe] + dir[axe] * ratio;
		}
		float dist = glm::distance(pos, inter);
		if (dist < minDist) {
			minDist = dist;
			nearestInter = inter;
		}
	}
	return { nearestInter, minDist };
}
