#include "Structure.h"

#include <cmath>

ivec2 Structure::getCenterPos(ivec2 globalPos) const {
	ivec3 size_ = size();
	return globalPos + ivec2{ size_.x, size_.z } / 2;
}
