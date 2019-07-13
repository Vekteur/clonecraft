#pragma once

#include "Structure.h"

class Fir : public Structure {
public:
	Fir();
	virtual bool isValidPos(ivec2 pos) const override;
};