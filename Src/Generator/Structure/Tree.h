#pragma once

#include "Structure.h"

class Tree : public Structure {
public:
	Tree();
	virtual bool isValidPos(ivec3 centerPos, BiomeID biomeID) const override;
};