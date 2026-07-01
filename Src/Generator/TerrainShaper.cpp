#include "TerrainShaper.h"

#include "Generator/Biome/BiomeMap.h"
#include "Maths/Converter.h"
#include "Maths/MiscMath.h"
#include "Util/DynamicArray3D.h"
#include "World/Chunk.h"

#include <algorithm>
#include <cmath>

namespace {
	// One block along a column: solid terrain, solid but carved away into a cave, or air.
	enum class ColumnBlock { AIR, CARVED, SOLID };

	// Walks a column from y = top down to 0. The surface sits where density crosses zero; the noise
	// term lets it fold back over itself into cliffs and overhangs. sampleNoise(pos) gives the density
	// noise (each caller samples it its own way), isCarved(pos, depth) decides whether a solid block is
	// hollowed out, and visit(pos, depth, kind) acts on the block. Returning false from visit stops the
	// walk. depth is the count of solid blocks already seen above this one in the column.
	template<typename SampleNoise, typename IsCarved, typename Visit>
	void walkColumn(int top, int height, double ruggedness, ivec2 xz,
			SampleNoise sampleNoise, IsCarved isCarved, Visit visit) {
		int depth = 0;
		for (int y = top; y >= 0; --y) {
			ivec3 pos{ xz.x, y, xz.y };
			double density = (height - y) + sampleNoise(pos) * ruggedness;
			if (density > 0.) {
				ColumnBlock kind = isCarved(pos, depth) ? ColumnBlock::CARVED : ColumnBlock::SOLID;
				if (!visit(pos, depth, kind))
					return;
				++depth;
			} else {
				depth = 0;
				if (!visit(pos, depth, ColumnBlock::AIR))
					return;
			}
		}
	}
}

TerrainShaper::TerrainShaper(const BiomeMap& biomeMap) : m_biomeMap{ biomeMap } {}

void TerrainShaper::fillBlocks(Chunk& chunk, const ChunkGenerationInfo& info) const {
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
	const ivec3 L{ DENSITY_LATTICE_XZ, DENSITY_LATTICE_Y, DENSITY_LATTICE_XZ };
	int nx = Const::SECTION_SIDE / L.x + 1, nz = Const::SECTION_SIDE / L.z + 1, ny = topY / L.y + 2;
	DynamicArray3D<double> grid({ nx, ny, nz });
	auto at = [&](int i, int j, int k) -> double& { return grid.at({ i, j, k }); };
	for (int i = 0; i < nx; ++i)
		for (int k = 0; k < nz; ++k)
			for (int j = 0; j < ny; ++j) {
				dvec3 p{ origin.x + i * L.x, j * L.y * DENSITY_VERTICAL_SCALE, origin.z + k * L.z };
				at(i, j, k) = m_terrainNoise.getNoise(p);
			}

	CaveCarver::Grid caveGrid = m_caveCarver.sample(origin, topY);

	auto sampleNoise = [&](ivec3 pos) {
		math::LatticeCoord lc = math::latticeCoord(pos, L);
		return math::trilerp(grid, lc.cell, lc.frac);
	};

	ivec2 origin2D = Converter::to2D(origin);
	for (int x = 0; x < Const::SECTION_SIDE; ++x)
		for (int z = 0; z < Const::SECTION_SIDE; ++z) {
			ivec2 c{ x, z };
			const Biome& biome = m_biomeMap.getBiome(info.biome(c));
			int height = info.height(c);
			double rugged = info.ruggedness(c);
			CaveCarver::Column caveCol = m_caveCarver.column(origin2D + c, height);

			walkColumn(topY, height, rugged, c, sampleNoise,
				[&](ivec3 local, int depth) { return m_caveCarver.isCarved(caveGrid, caveCol, local, depth); },
				[&](ivec3 local, int depth, ColumnBlock kind) {
					// Carved blocks stay air (but still count as depth, so caves keep stone walls);
					// below a region's lava level they pool flat lava, and the rest is solid terrain.
					switch (kind) {
					case ColumnBlock::CARVED:
						if (local.y < caveCol.lavaLevel)
							chunk.setBlock(local, { BlockID::LAVA });
						break;
					case ColumnBlock::SOLID:
						chunk.setBlock(local, biome.getBlock(origin + local, depth));
						break;
					case ColumnBlock::AIR:
						if (local.y < Const::SEA_LEVEL) {
							BlockID fluid = BlockID::WATER;
							if (local.y == Const::SEA_LEVEL - 1)
								fluid = biome.surfaceFluid();
							chunk.setBlock(local, { fluid });
						}
						break;
					}
					return true;
				});
		}
}

std::optional<int> TerrainShaper::surfaceHeight(ivec2 xz, const ColumnInfo& col) const {
	CaveCarver::Column caveCol = m_caveCarver.column(xz, col.height);
	// Solid can reach a bit above the base height where ruggedness lifts the surface; start safely above.
	int top = col.height + static_cast<int>(std::ceil(std::abs(col.ruggedness))) + 2;
	std::optional<int> result;
	walkColumn(top, col.height, col.ruggedness, xz,
		[&](ivec3 pos) {
			return m_terrainNoise.getNoise(dvec3{ double(pos.x), pos.y * DENSITY_VERTICAL_SCALE, double(pos.z) });
		},
		[&](ivec3 pos, int depth) { return m_caveCarver.isCarvedPointwise(caveCol, pos, depth); },
		[&](ivec3 pos, int, ColumnBlock kind) {
			if (kind == ColumnBlock::SOLID) {
				result = pos.y + 1; // first air block above the topmost solid block
				return false;
			}
			return true;
		});
	return result;
}
