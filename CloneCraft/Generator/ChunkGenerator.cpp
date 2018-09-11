#include "ChunkGenerator.h"

#include "Chunk.h"
#include "Structure.h"
#include "Tree.h"
#include "Converter.h"
#include "Logger.h"

#include <memory>
#include <optional>
#include <random>

ChunkGenerator::ChunkGenerator(Chunk& chunk)
	: chunk{ chunk }, m_position{ chunk.getPosition() } { }

double ChunkGenerator::getNoise(ivec2 pos) const {
	return perlin.getNoise(vec2{ static_cast<float>(pos.x) / 256, static_cast<float>(pos.y) / 256 });;
}

int ChunkGenerator::getHeight(double noise) const {
	return Const::SEA_LEVEL + 1 +
		floor(noise * 32);
}

Block ChunkGenerator::getBlock(int y, int height) const {
	if (y < height - 4)
		return { ID::STONE };
	if (y < Const::SEA_LEVEL + 1 && height - 3 <= y && y <= height - 1)
		return { ID::SAND };
	if (y < height - 1)
		return { ID::DIRT };
	if (y == height - 1)
		return { ID::GRASS };
	if (y < Const::SEA_LEVEL)
		return { ID::WATER };

	return { ID::AIR };
}

void ChunkGenerator::loadNoise() {
	for (int x = 0; x < Const::SECTION_SIDE; ++x)
		for (int z = 0; z < Const::SECTION_SIDE; ++z) {
			ivec2 chunkPos = ivec2{ x, z } + m_position * Const::SECTION_SIDE;
			m_noise[x][z] = getNoise(chunkPos);
		}
}

void ChunkGenerator::loadBlocks() {
	for (int x = 0; x < Const::SECTION_SIDE; ++x)
		for (int z = 0; z < Const::SECTION_SIDE; ++z) {
			int height = getHeight(m_noise[x][z]);
			for (int y = 0; y < Const::INIT_CHUNK_HEIGHT; ++y) {
				ivec3 pos{ x, y, z };
				Block block = getBlock(pos.y, height);
				chunk.setBlock(pos, block);
			}
		}
}

void ChunkGenerator::loadStructures() {
	std::vector<std::unique_ptr<Structure>> structures;
	structures.push_back(std::make_unique<Tree>());

	for (auto& structure : structures) {
		ivec3 size = structure->size();
		ivec2 zoneSize = { size.x, size.z };

		ivec2 lowerZone = m_position * Const::SECTION_SIDE + 1 - zoneSize;
		lowerZone = { floorDiv(lowerZone.x, size.x), floorDiv(lowerZone.y, size.z) };
		ivec2 upperZone = (m_position + 1) * Const::SECTION_SIDE - 1;
		upperZone = { floorDiv(upperZone.x, size.x), floorDiv(upperZone.y, size.z) };
		for (int zoneX = lowerZone.x; zoneX <= upperZone.x; ++zoneX) {
			for (int zoneY = lowerZone.y; zoneY <= upperZone.y; ++zoneY) {
				ivec2 zonePos = { zoneX, zoneY };
				std::optional<ivec3> optStructPos = structure->getLocalPos(zonePos, { 0.25f, 0.25f }, chunk);
				if (optStructPos.has_value()) {
					ivec3 structPos = optStructPos.value();
					ivec3 globalPos = { zonePos.x * size.x + structPos.x, structPos.y, zonePos.y * size.z + structPos.z };

					for (int x = 0; x < size.x; ++x)
						for (int z = 0; z < size.z; ++z)
							for (int y = 0; y < size.y; ++y) {
								ivec3 localPos{ x, y, z };
								Block block = structure->getBlock(localPos);
								ivec3 pos = globalPos + localPos;
								ivec3 innerChunkPos = pos - Converter::chunkToGlobal(m_position);
								if (block.id != +ID::AIR && chunk.isInChunk(innerChunkPos)) {
									chunk.setBlock(innerChunkPos, block);
								}
							}
				}
			}
		}
	}
}

void ChunkGenerator::load() {
	loadNoise();
	loadBlocks();
	loadStructures();
}