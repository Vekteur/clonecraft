#pragma once

#include "Structure.h"
#include "Util/Array3D.h"
#include "Maths/GlmCommon.h"
#include "Block/Block.h"

class Tree : public Structure {
public:
	Tree();
	virtual std::optional<ivec3> getLocalPos(ivec2 zonePos, vec2 freq, const Chunk& chunk) const override;
	virtual Block getBlock(ivec3 pos) const override;
	virtual ivec3 size() const override;

private:
	static const int s_sizeX = 5, s_sizeY = 7, s_sizeZ = 5;
	static const ivec3 s_size;
	Array3D<Block, s_sizeX, s_sizeY, s_sizeZ> m_blocks;
};

