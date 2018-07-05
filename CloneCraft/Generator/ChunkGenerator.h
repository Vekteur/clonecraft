#pragma once

#include "OctavePerlin.h"
#include "WorldConstants.h"
#include "Converter.h"

class ChunkGenerator
{
public:
	ChunkGenerator(ivec2 pos);
	~ChunkGenerator();

	int getBlock(ivec3 globalPos);

private:
	ivec2 m_position;

	OctavePerlin m_perlin{ 4, 0.5, 4.0};
	float m_noise[Const::SECTION_SIDE][Const::SECTION_SIDE];
};

