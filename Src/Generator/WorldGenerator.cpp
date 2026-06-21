#include "WorldGenerator.h"

#include "Maths/Converter.h"
#include "Maths/MiscMath.h"
#include "Generator/Noise/PerlinNoise.h"
#include "Util/DynamicArray3D.h"
#include "Util/Logger.h"

#include <algorithm>
#include <cmath>

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
			ColumnInfo col = m_biomeMap.getColumn(pos);
			chunk.chunkInfo().biome(innerPos) = col.biome;
			chunk.chunkInfo().height(innerPos) = col.height;
			chunk.chunkInfo().ruggedness(innerPos) = col.ruggedness;
		}
}

void WorldGenerator::loadBlocks(Chunk& chunk) const {
	const ChunkGenerationInfo& info = chunk.chunkInfo();
	ivec3 origin = Converter::chunkToGlobal(chunk.getPosition());

	// Highest block we might place: tallest column plus how far its overhangs can reach up.
	int topY = Const::SEA_LEVEL;
	for (int x = 0; x < Const::SECTION_SIDE; ++x)
		for (int z = 0; z < Const::SECTION_SIDE; ++z) {
			ivec2 c{ x, z };
			topY = std::max(topY, info.height(c) + static_cast<int>(ceil(info.ruggedness(c))));
		}
	topY += 1;

	// Sample the 3D noise on a coarse lattice, then trilinearly interpolate it per block below.
	const int LXZ = DENSITY_LATTICE_XZ, LY = DENSITY_LATTICE_Y;
	int nx = Const::SECTION_SIDE / LXZ + 1, nz = Const::SECTION_SIDE / LXZ + 1, ny = topY / LY + 2;
	DynamicArray3D<double> grid({ nx, ny, nz });
	auto at = [&](int i, int j, int k) -> double& { return grid.at({ i, j, k }); };
	for (int i = 0; i < nx; ++i)
		for (int k = 0; k < nz; ++k)
			for (int j = 0; j < ny; ++j) {
				dvec3 p{ origin.x + i * LXZ, j * LY * DENSITY_VERTICAL_SCALE, origin.z + k * LXZ };
				at(i, j, k) = m_terrainNoise.getNoise(p);
			}

	CaveCarver::Grid caveGrid = m_caveCarver.sample(origin, topY);

	auto sampleNoise = [&](int bx, int by, int bz) {
		ivec3 cell{ bx / LXZ, by / LY, bz / LXZ };
		dvec3 frac{ (bx % LXZ) / double(LXZ), (by % LY) / double(LY), (bz % LXZ) / double(LXZ) };
		return math::trilerp(grid, cell, frac);
	};

	for (int x = 0; x < Const::SECTION_SIDE; ++x)
		for (int z = 0; z < Const::SECTION_SIDE; ++z) {
			ivec2 c{ x, z };
			const Biome& biome = m_biomeMap.getBiome(info.biome(c));
			int height = info.height(c);
			double rugged = info.ruggedness(c);
			CaveCarver::Column caveCol = m_caveCarver.column({ origin.x + x, origin.z + z }, height);

			// Walk the column top to bottom. The surface sits where density crosses zero; the noise
			// term lets it fold back over itself into cliffs and overhangs.
			int depth = 0; // solid blocks already placed above this one in the column
			for (int y = topY; y >= 0; --y) {
				double density = (height - y) + sampleNoise(x, y, z) * rugged;
				ivec3 local{ x, y, z };
				if (density > 0.) {
					// Carved blocks stay air but still count as depth, so caves keep stone walls.
					if (!m_caveCarver.isCarved(caveGrid, caveCol, local, origin + local, depth))
						chunk.setBlock(local, biome.getBlock(origin + local, depth));
					++depth;
				}
				else {
					depth = 0;
					if (y < Const::SEA_LEVEL) {
						BlockID fluid = BlockID::WATER;
						if (y == Const::SEA_LEVEL - 1)
							fluid = biome.surfaceFluid();
						chunk.setBlock(local, { fluid });
					}
				}
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
				ivec2 supportPos2D = structure.getSupportPos(globalPos2D);
				BiomeID posBiomeID = biomeMap().getBiomeID(supportPos2D);
				if (posBiomeID != biomeID)
					continue;
				ivec3 supportPos = { supportPos2D.x, biomeMap().getHeight(supportPos2D), supportPos2D.y };
				if (!structure.isValidPos(supportPos, posBiomeID))
					continue;
				ivec3 globalPos = { globalPos2D.x, supportPos.y, globalPos2D.y };
				drawStructure(chunk, structure, globalPos);
			}
		}
	}
}

std::optional<ivec2> WorldGenerator::findLocalPos(const Structure& structure, ivec2 zonePos, float freq) const {
	ivec3 size = structure.size();
	ivec2 zoneSize = { size.x, size.z };
	ivec2 bounds = float(1.f / sqrt(freq)) * vec2(zoneSize);

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