#include "WorldGenerator.h"

#include "Maths/Converter.h"
#include "Generator/Noise/PerlinNoise.h"
#include "Util/Logger.h"

#include <algorithm>

void WorldGenerator::loadChunk(Chunk& chunk) const {
	loadHeights(chunk);
	loadBlocks(chunk);
	loadStructures(chunk);
}

const BiomeMap& WorldGenerator::biomeMap() const {
	return m_biomeMap;
}

void WorldGenerator::loadHeights(Chunk& chunk) const {
	for (int x = 0; x < Const::SECTION_SIDE; ++x)
		for (int z = 0; z < Const::SECTION_SIDE; ++z) {
			ivec2 innerPos{ x, z };
			ivec2 pos = innerPos + chunk.getPosition() * Const::SECTION_SIDE;
			BiomeID biomeID = m_biomeMap.getBiomeID(pos);
			int height = m_biomeMap.getHeight(pos);
			chunk.chunkInfo().biome(innerPos) = biomeID;
			chunk.chunkInfo().height(innerPos) = height;
		}
}

void WorldGenerator::loadBlocks(Chunk& chunk) const {
	for (int x = 0; x < Const::SECTION_SIDE; ++x)
		for (int z = 0; z < Const::SECTION_SIDE; ++z) {
			ivec2 pos2D{ x, z };
			const Biome& biome = m_biomeMap.getBiome(chunk.chunkInfo().biome(pos2D));
			int height = chunk.chunkInfo().height(pos2D);
			for (int y = 0; y < Const::INIT_CHUNK_HEIGHT; ++y) {
				ivec3 pos{ x, y, z };
				Block block = biome.getBlock(pos.y, height);
				chunk.setBlock(pos, block);
			}
		}
}

void WorldGenerator::loadStructures(Chunk& chunk) const {
	// To improve : avoid iterating over all biomes
	for (int biomeID = 0; biomeID < static_cast<int>(BiomeID::SIZE); ++biomeID) {
		const Biome& biome = m_biomeMap.getBiome(static_cast<BiomeID>(biomeID));
		for (StructureInfo structInfo : biome.getStructures()) {
			const Structure& structure = m_biomeMap.getStructure(structInfo.id);
			loadStructure(chunk, structure, structInfo.freq, static_cast<BiomeID>(biomeID));
		}
	}
}

void WorldGenerator::loadStructure(Chunk& chunk, const Structure& structure, float freq, BiomeID biomeID) const {
	ivec3 size = structure.size();
	ivec2 zoneSize = { size.x, size.z };

	ivec2 lowerZone = chunk.getPosition() * Const::SECTION_SIDE + 1 - zoneSize;
	lowerZone = { floorDiv(lowerZone.x, size.x), floorDiv(lowerZone.y, size.z) };
	ivec2 upperZone = (chunk.getPosition() + 1) * Const::SECTION_SIDE - 1;
	upperZone = { floorDiv(upperZone.x, size.x), floorDiv(upperZone.y, size.z) };
	for (int zoneX = lowerZone.x; zoneX <= upperZone.x; ++zoneX) {
		for (int zoneY = lowerZone.y; zoneY <= upperZone.y; ++zoneY) {
			ivec2 zonePos = { zoneX, zoneY };
			std::optional<ivec2> optLocalPos = findLocalPos(structure, zonePos, freq);
			if (optLocalPos.has_value()) {
				ivec2 localPos = optLocalPos.value();
				ivec2 globalPos2D = zonePos * zoneSize + localPos;
				if (!structure.isValidPos(globalPos2D) ||
						biomeMap().getBiomeID(structure.getCenterPos(globalPos2D)) != biomeID)
					continue;
				ivec3 globalPos = { globalPos2D.x, biomeMap().getHeight(globalPos2D), globalPos2D.y };
				drawStructure(chunk, structure, globalPos);
			}
		}
	}
}

std::optional<ivec2> WorldGenerator::findLocalPos(const Structure& structure, ivec2 zonePos, float freq) const {
	ivec3 size = structure.size();
	ivec2 zoneSize = { size.x, size.z };
	ivec2 bounds = (1.f / sqrt(freq)) * vec2(zoneSize);

	int posX = PerlinNoise::perm[PerlinNoise::perm[posMod(zonePos.x, 256)] + posMod(zonePos.y, 256)];
	int posY = PerlinNoise::perm[PerlinNoise::perm[posMod(zonePos.y, 256)] + posMod(zonePos.x, 256)];
	ivec2 pos = ivec2{ posX, posY } % bounds;

	if (pos.x < zoneSize.x && pos.y < zoneSize.y)
		return pos;
	return std::nullopt;
}

void WorldGenerator::drawStructure(Chunk& chunk, const Structure& structure, ivec3 globalPos) const {
	ivec3 size = structure.size();
	ivec2 size2D = Converter::to2D(size);
	ivec3 globalChunkPos = Converter::chunkToGlobal(chunk.getPosition());
	ivec2 globalPos2D = Converter::to2D(globalPos), globalChunkPos2D = Converter::to2D(globalChunkPos);
	ivec2 minPos = max(globalChunkPos2D, globalPos2D);
	ivec2 maxPos = min(globalChunkPos2D + Const::SECTION_SIDE, globalPos2D + size2D);
	for (int x = minPos.x; x < maxPos.x; ++x)
		for (int z = minPos.y; z < maxPos.y; ++z)
			for (int y = globalPos.y; y < globalPos.y + size.y; ++y) {
				ivec3 pos{ x, y, z };
				ivec3 localPos = pos - globalPos;
				Block block = structure.getBlock(localPos);
				if (block.id != +BlockID::AIR) {
					ivec3 innerChunkPos = pos - globalChunkPos;
					chunk.setBlock(innerChunkPos, block);
				}
			}
}

WorldGenerator g_worldGenerator;