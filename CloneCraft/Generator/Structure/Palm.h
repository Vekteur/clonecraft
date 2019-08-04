#pragma once

#include "Structure.h"

class Palm : public Structure {
public:
	Palm();
	virtual bool isValidPos(ivec3 centerPpos, BiomeID biomeID) const override;
};