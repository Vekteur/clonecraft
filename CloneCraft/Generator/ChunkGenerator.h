#pragma once

#include "OctavePerlin.h"
#include "WorldConstants.h"
#include "Converter.h"
#include "Block.h"

class ChunkGenerator
{
public:
	ChunkGenerator(ivec2 pos);

	Block getBlock(ivec3 globalPos);

private:
	ivec2 m_position;
	float m_noise[Const::SECTION_SIDE][Const::SECTION_SIDE];
};

