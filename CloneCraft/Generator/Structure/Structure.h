#pragma once

#include "Array3D.h"
#include "Block.h"

#include "GlmCommon.h"

#include <optional>

class Structure {
public:
	Structure();
	virtual std::optional<ivec3> getLocalPos(ivec3 globalPos) const = 0;

private:
	//Array3D<Block> m_blocks;
};

