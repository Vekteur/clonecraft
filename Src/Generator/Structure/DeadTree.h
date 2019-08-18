#pragma once

#include "Structure.h"

class DeadTree : public Structure {
public:
	DeadTree();
	virtual bool isValidPos(ivec3 centerPos, BiomeID biomeID) const override;
};