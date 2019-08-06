#pragma once

#include "Structure.h"

class Palm : public Structure {
public:
	Palm();
	virtual ivec2 getSupportPos(ivec2 globalPos) const override;
	virtual bool isValidPos(ivec3 centerPpos, BiomeID biomeID) const override;
};