#pragma once

#include "Structure.h"

// A maple: a straight trunk under a dense, rounded crown of autumn leaves, grown procedurally from
// its world position so every maple is distinct.
class Maple : public Structure {
public:
	Maple();
	DynamicArray3D<Block> build(uint32_t seed) const override;
	bool isValidPos(ivec3 centerPos, BiomeID biomeID) const override;
};
