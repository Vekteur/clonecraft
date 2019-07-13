#pragma once

#include "Util/DynamicArray3D.h"
#include "Block/Block.h"

#include "Maths/GlmCommon.h"

#include <optional>

class Structure {
public:
	Structure(ivec3 size);
	virtual ~Structure() = default;
	virtual bool isValidPos(ivec2 pos) const = 0;
	Block getBlock(ivec3 pos) const;
	ivec3 size() const;
	ivec2 getCenterPos(ivec2 globalPos = ivec2{ 0, 0 }) const;

protected:
	DynamicArray3D<Block> m_blocks;

	void add(ivec3 pos, Block block);
	void addSymetrically(ivec3 pos, Block block);
	void fill(ivec3 low, ivec3 high, Block block);
	void fillSymetrically(ivec3 low, ivec3 high, Block block);
};

