#include "ChunkGenerator.h"

ChunkGenerator::ChunkGenerator(ivec2 pos) : m_position{ pos } {
	for (int x = 0; x < Const::SECTION_SIDE; ++x)
		for (int z = 0; z < Const::SECTION_SIDE; ++z) {
			ivec2 chunkPos = ivec2{ x, z } + m_position * Const::CHUNK_SIDE;
			m_noise[x][z] = m_perlin.getNoise(vec2{ static_cast<float>(chunkPos.x) / 256, static_cast<float>(chunkPos.y) / 256 });
		}
}

ChunkGenerator::~ChunkGenerator() {
}

int ChunkGenerator::getBlock(ivec3 globalPos) {
	int height = Const::SEA_LEVEL +
		floor(m_noise[posMod(globalPos.x, Const::SECTION_SIDE)][posMod(globalPos.z, Const::SECTION_SIDE)] * 16);
	if (globalPos.y <= height)
		return 1;

	return 0;
}