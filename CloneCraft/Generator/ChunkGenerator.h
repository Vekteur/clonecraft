#pragma once

#include "OctavePerlin.h"
#include "WorldConstants.h"
#include "Converter.h"
#include "Block.h"
#include "WorldConstants.h"

#include <array>

class Chunk;
class ChunkGenerator {
public:
	ChunkGenerator(Chunk& chunk);

	void loadNoise();
	void load();

private:
	Block getBlock(ivec3 globalPos) const;

	Chunk& chunk;
	ivec2 m_position;
	float m_noise[Const::SECTION_SIDE][Const::SECTION_SIDE];
};

