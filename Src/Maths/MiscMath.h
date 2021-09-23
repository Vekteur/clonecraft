#pragma once

#include "GlmCommon.h"

namespace math
{
	int manhattan(ivec2 pos1, ivec2 pos2) {
		return std::abs(pos2.x - pos1.x) + std::abs(pos2.y - pos1.y);
	}

	int euclidianPow2(ivec2 pos1, ivec2 pos2) {
		int dx = pos1.x - pos2.x;
		int dy = pos1.y - pos2.y;
		return dx * dx + dy * dy;
	}
}