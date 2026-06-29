#pragma once

#include "Structure.h"

// A Japanese cherry: a short dark trunk that splits into branches spreading wide into a broad,
// flat-topped pink canopy. Grown procedurally from its world position so every tree is distinct.
class Cherry : public Structure {
public:
	Cherry();
	DynamicArray3D<Block> build(uint32_t seed) const override;
	bool isValidPos(ivec3 centerPos, BiomeID biomeID) const override;
};
