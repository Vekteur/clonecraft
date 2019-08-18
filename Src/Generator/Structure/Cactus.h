#pragma once

#include "Structure.h"

class Cactus : public Structure {
public:
	Cactus();
	virtual bool isValidPos(ivec3 centerPos, BiomeID biomeID) const override;
};