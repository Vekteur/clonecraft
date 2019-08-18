#pragma once

#include "Structure.h"

class Fir : public Structure {
public:
	Fir();
	virtual bool isValidPos(ivec3 centerPos, BiomeID biomeID) const override;
};