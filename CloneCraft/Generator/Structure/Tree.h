#pragma once

#include "Structure.h"
#include "Array3D.h"
#include "GlmCommon.h"
#include "Block.h"

class Tree : public Structure {
public:
	Tree();
	virtual std::optional<ivec3> getLocalPos(ivec2 zonePos, vec2 freq, const Chunk& chunk) const override;
	virtual Block getBlock(ivec3 pos) const override;
	virtual ivec3 size() const override { return s_size; }

private:
	static const int s_sizeX = 7, s_sizeY = 9, s_sizeZ = 7;
	static const ivec3 s_size;
	Array3D<Block, s_sizeX, s_sizeY, s_sizeZ> m_blocks;
};

