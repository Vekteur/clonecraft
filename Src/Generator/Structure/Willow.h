#pragma once

#include "Structure.h"

// A weeping willow: a wide, low crown with leaf strands drooping from the branch tips. Each one is
// grown procedurally from its world position, so every willow is distinct.
class Willow : public Structure {
public:
	Willow();
	DynamicArray3D<Block> build(uint32_t seed) const override;
	bool isValidPos(ivec3 centerPos, BiomeID biomeID) const override;
};
