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

	void load();

	int getHeight(double noise) const;
	Block getBlock(int y, int height) const;
	double getNoise(ivec2 pos) const;

private:
	void loadNoise();
	void loadBlocks();
	void loadStructures();

	Chunk& chunk;
	ivec2 m_position;
	OctavePerlin perlin{ 4, 0.5, 2.0 };
	double m_noise[Const::SECTION_SIDE][Const::SECTION_SIDE];
};

