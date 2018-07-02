#include "ChunkGenerator.h"

ChunkGenerator::ChunkGenerator(ivec2 pos) : m_position{ pos } {
	for (int x = 0; x < Const::SECTION_SIDE; ++x)
		for (int z = 0; z < Const::SECTION_SIDE; ++z)
			m_noise[x][z] = m_perlin.getNoise(vec2{ static_cast<float>(x + m_position.x * Const::CHUNK_SIDE) / Const::SECTION_SIDE, static_cast<float>(z + m_position.y * Const::CHUNK_SIDE) / Const::SECTION_SIDE });
}

ChunkGenerator::~ChunkGenerator() {
}

int ChunkGenerator::getBlock(ivec3 globalPos) {
	int height = Const::SEA_LEVEL +
		floor(m_noise[Converter::positiveMod(globalPos.x, Const::SECTION_SIDE)][Converter::positiveMod(globalPos.z, Const::SECTION_SIDE)] * 4);
	if (globalPos.y <= height)
		return 1;
	else
		return 0;

	return 0;
}