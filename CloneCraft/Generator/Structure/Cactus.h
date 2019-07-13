#pragma once

#include "Structure.h"

class Cactus : public Structure {
public:
	Cactus();
	virtual bool isValidPos(ivec2 pos) const override;
};