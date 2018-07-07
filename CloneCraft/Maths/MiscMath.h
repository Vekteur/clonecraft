#pragma once

#include "GlmCommon.h"

namespace math
{
	int manhattan(ivec2 pos1, ivec2 pos2) {
		return abs(pos2.x - pos1.x) + abs(pos2.y - pos1.y);
	}
}