#pragma once

#include "Structure.h"
#include "Util/Array3D.h"
#include "Maths/GlmCommon.h"
#include "Block/Block.h"

class Tree : public Structure {
public:
	Tree();
	virtual bool isValidPos(ivec2 pos) const override;
	virtual Block getBlock(ivec3 pos) const override;
	virtual ivec3 size() const override;

private:
	static const int s_sizeX = 5, s_sizeY = 7, s_sizeZ = 5;
	static const ivec3 s_size;
	Array3D<Block, s_sizeX, s_sizeY, s_sizeZ> m_blocks;

	void fill(ivec3 low, ivec3 high, Block block);
};

