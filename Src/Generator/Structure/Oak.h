#pragma once

#include "Structure.h"

class Oak : public Structure {
public:
	Oak();
	virtual bool isValidPos(ivec3 centerPos, BiomeID biomeID) const override;
};