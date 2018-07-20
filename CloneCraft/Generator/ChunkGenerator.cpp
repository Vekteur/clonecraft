#include "ChunkGenerator.h"

ChunkGenerator::ChunkGenerator(ivec2 pos) : m_position{ pos } {
	OctavePerlin perlin{ 4, 0.5, 2.0 };
	for (int x = 0; x < Const::SECTION_SIDE; ++x)
		for (int z = 0; z < Const::SECTION_SIDE; ++z) {
			ivec2 chunkPos = ivec2{ x, z } + m_position * Const::CHUNK_SIDE;
			m_noise[x][z] = perlin.getNoise(vec2{ static_cast<float>(chunkPos.x) / 256, static_cast<float>(chunkPos.y) / 256 });
		}
}

Block ChunkGenerator::getBlock(ivec3 globalPos) {
	int height = Const::SEA_LEVEL + 1 +
		floor(m_noise[posMod(globalPos.x, Const::SECTION_SIDE)][posMod(globalPos.z, Const::SECTION_SIDE)] * 32);
	if (globalPos.y < height - 4)
		return { ID::STONE };
	if (globalPos.y < Const::SEA_LEVEL + 1 && height - 3 <= globalPos.y && globalPos.y <= height - 1)
		return { ID::SAND };
	if (globalPos.y < height - 1)
		return { ID::DIRT };
	if (globalPos.y == height - 1)
		return { ID::GRASS };
	if (globalPos.y < Const::SEA_LEVEL)
		return { ID::WATER };

	return { ID::AIR };
}