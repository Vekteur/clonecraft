#pragma once

#include "Structure.h"

// A large oak grown procedurally from its world position, so every instance has a distinct
// trunk lean, branch layout and canopy. The size is a fixed bounding box; each tree fills only
// part of it and the rest stays air.
class BigOak : public Structure {
public:
	BigOak();
	DynamicArray3D<Block> build(uint32_t seed) const override;
	bool isValidPos(ivec3 centerPos, BiomeID biomeID) const override;
};
