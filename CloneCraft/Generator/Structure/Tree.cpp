#include "Tree.h"

#include "ChunkMap.h"
#include "Logger.h"
#include "PerlinNoise.h"

#include <random>

const ivec3 Tree::s_size{ s_sizeX, s_sizeY, s_sizeZ };

Tree::Tree() : Structure() {
	ivec2 center = ivec2{ size().x, size().z } / 2;
	for (int y = 3; y <= 4; ++y)
		for (int x = center.x - 2; x <= center.x + 2; ++x)
			for (int z = center.y - 2; z <= center.y + 2; ++z)
				m_blocks.at({ x, y, z }) = +ID::LEAVES;

	for (int y = 5; y <= 6; ++y)
		for (int x = center.x - 1; x <= center.x + 1; ++x)
			for (int z = center.y - 1; z <= center.y + 1; ++z)
				m_blocks.at({ x, y, z }) = +ID::LEAVES;

	for (int y = 0; y <= 4; ++y)
		m_blocks.at({ center.x, y, center.y }) = +ID::LOG;
}

std::optional<ivec3> Tree::getLocalPos(ivec2 zonePos, vec2 freq, const Chunk& chunk) const {

	ivec2 zoneSize = { size().x, size().z };
	ivec2 bounds = (1.f / freq) * vec2(zoneSize);

	int posX = PerlinNoise::perm[PerlinNoise::perm[posMod(zonePos.x, 256)] + posMod(zonePos.y, 256)];
	int posY = PerlinNoise::perm[posX];
	ivec2 pos = { posX % bounds.x, posY % bounds.y };

	if (pos.x < zoneSize.x && pos.y < zoneSize.y) {
		ivec2 centerPos = zonePos * zoneSize + pos + zoneSize / 2;
		double noise = chunk.getChunkGenerator().getNoise(centerPos);
		int height = chunk.getChunkGenerator().getHeight(noise);
		if (chunk.getChunkGenerator().getBlock(height - 1, height).id == +ID::GRASS) {
			return ivec3{ pos.x, height, pos.y };
		}
	}
	return std::nullopt;
}

Block Tree::getBlock(ivec3 pos) const {
	return m_blocks.at(pos);
}

ivec3 Tree::size() const { 
	return s_size; 
}