#include "Tree.h"

#include "ChunkMap.h"
#include "Logger.h"

#include <random>

const ivec3 Tree::s_size{ s_sizeX, s_sizeY, s_sizeZ };

Tree::Tree() : Structure() {
	for (int y = 0; y < s_size.y; ++y) {
		m_blocks.at({ size().x / 2, y, size().z / 2 }) = +ID::LOG;
	}
}

std::optional<ivec3> Tree::getLocalPos(ivec2 zonePos, vec2 freq, const Chunk& chunk) const {
	ivec2 zoneSize = { size().x, size().z };
	ivec2 bounds = (1.f / freq) * vec2(zoneSize);
	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<int> distX(0, bounds.x - 1);
	std::uniform_int_distribution<int> distY(0, bounds.y - 1);

	ivec2 pos = { distX(rng), distY(rng) };
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
