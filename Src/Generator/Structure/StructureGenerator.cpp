#include "StructureGenerator.h"

#include "Generator/Biome/BiomeMap.h"
#include "Generator/TerrainShaper.h"
#include "Maths/Converter.h"
#include "Maths/MiscMath.h"
#include "World/Chunk.h"

#include <algorithm>

namespace {
	// Structure grids, in collision priority order: big first, then medium, then small. Within a size,
	// a base grid plus three half-cell offsets, also in a fixed order, so a structure can land at any
	// position. A structure fits in a cell of its tier, so two structures in the same grid never overlap.
	// Collisions are checked wrt grids of higher priority.
	constexpr int NUM_OFFSETS = 4;
	constexpr int SMALL_CELL = 10, MEDIUM_CELL = 32, BIG_CELL = 100;
	constexpr int SMALL_MAX = 5, MEDIUM_MAX = 20, BIG_MAX = 50;

	// Independent hash streams for one cell of one grid.
	enum CellSalt { SALT_SELECT = 1, SALT_POS_X = 2, SALT_POS_Z = 3 };

	// Records the world columns a built structure occupies, plus their bounding box, for collisions.
	void computeFootprint(Placement& p) {
		ivec3 size = p.blocks.size();
		ivec2 origin2D = Converter::to2D(p.origin);
		bool any = false;
		for (int x = 0; x < size.x; ++x)
			for (int z = 0; z < size.z; ++z) {
				bool occupied = false;
				for (int y = 0; y < size.y && !occupied; ++y)
					occupied = p.blocks.at({ x, y, z }).id != +BlockID::AIR;
				if (!occupied)
					continue;
				ivec2 col = origin2D + ivec2{ x, z };
				p.columns.insert(col);
				if (!any) {
					p.footMin = p.footMax = col;
					any = true;
				} else {
					p.footMin = glm::min(p.footMin, col);
					p.footMax = glm::max(p.footMax, col);
				}
			}
	}

	// Do two placements share any column? An AABB test rejects the common case cheaply first.
	bool overlaps(const Placement& a, const Placement& b) {
		if (a.footMax.x < b.footMin.x || b.footMax.x < a.footMin.x ||
			a.footMax.y < b.footMin.y || b.footMax.y < a.footMin.y)
			return false;
		const auto& small = a.columns.size() <= b.columns.size() ? a.columns : b.columns;
		const auto& large = a.columns.size() <= b.columns.size() ? b.columns : a.columns;
		for (const ivec2& col : small)
			if (large.find(col) != large.end())
				return true;
		return false;
	}
}

StructureGenerator::StructureGenerator(const BiomeMap& biomeMap, const TerrainShaper& terrain)
		: m_biomeMap{ biomeMap }, m_terrain{ terrain } {
	// Define the structure grids in collision priority order.
	struct TierDef { int cell; StructureTier tier; int maxSize; };
	const TierDef tiers[]{
		{ BIG_CELL, StructureTier::BIG, BIG_MAX },
		{ MEDIUM_CELL, StructureTier::MEDIUM, MEDIUM_MAX },
		{ SMALL_CELL, StructureTier::SMALL, SMALL_MAX },
	};
	for (const TierDef& t : tiers) {
		int h = t.cell / 2;
		const ivec2 offsets[NUM_OFFSETS]{ { 0, 0 }, { 0, h }, { h, 0 }, { h, h } };
		for (const ivec2& offset : offsets)
			m_grids.push_back(std::make_unique<StructureGrid>(t.cell, offset, t.tier, t.maxSize));
	}
}

void StructureGenerator::place(Chunk& chunk) const {
	ivec3 chunkOrigin = Converter::chunkToGlobal(chunk.getPosition());
	ivec2 chunkMin = Converter::to2D(chunkOrigin);
	ivec2 chunkMaxIncl = chunkMin + (Const::SECTION_SIDE - 1);
	// Each grid only hosts structures fitting in one cell, so any structure touching the chunk lives in
	// a cell that intersects it. Draw order does not matter: which cells hold a structure is already
	// settled by resolveCell.
	for (int i = 0; i < static_cast<int>(m_grids.size()); ++i) {
		const StructureGrid& grid = *m_grids[i];
		ivec2 lo = grid.cellOf(chunkMin);
		ivec2 hi = grid.cellOf(chunkMaxIncl);
		for (int cx = lo.x; cx <= hi.x; ++cx)
			for (int cz = lo.y; cz <= hi.y; ++cz) {
				std::shared_ptr<const Placement> placement = resolveCell(i, { cx, cz });
				if (placement)
					drawPlacement(chunk, *placement);
			}
	}
}

std::shared_ptr<const Placement> StructureGenerator::resolveCell(int gridIndex, ivec2 cell) const {
	const StructureGrid& grid = *m_grids[gridIndex];
	if (std::optional<std::shared_ptr<const Placement>> cached = grid.get(cell))
		return *cached;
	// Compute outside the lock: the result is a pure function of the cell, so a racing thread that
	// computes the same cell gets the same answer and store() keeps whichever lands first.
	return grid.store(cell, computeCell(gridIndex, cell));
}

std::shared_ptr<const Placement> StructureGenerator::computeCell(int gridIndex, ivec2 cell) const {
	const StructureGrid& grid = *m_grids[gridIndex];
	int cellSize = grid.cellSize();
	ivec2 cellMin = grid.cellMin(cell);

	// Pick the structure from the biome at the cell center, then a position that keeps it inside the
	// cell. Frequencies are scaled so the world keeps the same density as a single zone per structure.
	ColumnInfo centerCol = m_biomeMap.getColumn(cellMin + cellSize / 2);
	double roll = math::fnvHash01({ gridIndex, cell.x, cell.y, SALT_SELECT });
	std::optional<StructureID> id = pickStructure(centerCol.biome, grid.tier(), double(cellSize) * cellSize, roll);
	if (!id.has_value())
		return nullptr;

	const Structure& structure = m_biomeMap.getStructure(*id);
	ivec3 size = structure.size();
	ivec2 range{ cellSize - size.x, cellSize - size.z };
	if (range.x <= 0 || range.y <= 0)
		return nullptr;
	int posX = int(math::fnvHash({ gridIndex, cell.x, cell.y, SALT_POS_X }) % unsigned(range.x));
	int posY = int(math::fnvHash({ gridIndex, cell.x, cell.y, SALT_POS_Z }) % unsigned(range.y));
	ivec2 pos2D = cellMin + ivec2{ posX, posY };

	ivec2 support2D = structure.getSupportPos(pos2D);
	ColumnInfo supportCol = m_biomeMap.getColumn(support2D);
	if (supportCol.biome != centerCol.biome) // drifted into another biome
		return nullptr;
	std::optional<int> baseY = m_terrain.surfaceHeight(support2D, supportCol);
	if (!baseY.has_value())
		return nullptr;
	if (!structure.isValidPos({ support2D.x, *baseY, support2D.y }, centerCol.biome))
		return nullptr;

	auto placement = std::make_shared<Placement>();
	placement->id = *id;
	placement->origin = { pos2D.x, *baseY, pos2D.y };
	placement->blocks = structure.build(math::fnvHash({ pos2D.x, pos2D.y }));
	computeFootprint(*placement);
	if (placement->columns.empty())
		return nullptr;
	if (collidesWithHigher(gridIndex, *placement))
		return nullptr;
	return placement;
}

bool StructureGenerator::collidesWithHigher(int gridIndex, const Placement& candidate) const {
	for (int j = 0; j < gridIndex; ++j) {
		const StructureGrid& grid = *m_grids[j];
		ivec2 lo = grid.cellOf(candidate.footMin);
		ivec2 hi = grid.cellOf(candidate.footMax);
		for (int cx = lo.x; cx <= hi.x; ++cx)
			for (int cz = lo.y; cz <= hi.y; ++cz) {
				std::shared_ptr<const Placement> other = resolveCell(j, { cx, cz });
				if (other && overlaps(*other, candidate))
					return true;
			}
	}
	return false;
}

std::optional<StructureID> StructureGenerator::pickStructure(BiomeID biome, StructureTier tier, double cellArea,
		double roll) const {
	double accum = 0.;
	for (const StructureInfo& info : m_biomeMap.getBiome(biome).getStructures()) {
		const Structure& structure = m_biomeMap.getStructure(info.id);
		ivec3 size = structure.size();
		if (tierOf(size) != tier)
			continue;
		// One zone per structure had area size.x * size.z and held the structure with probability freq,
		// so its density was freq / area. A cell of this tier offers NUM_OFFSETS independent chances
		// over cellArea, hence this per-cell probability reproduces the same density.
		double zoneArea = double(size.x) * size.z;
		accum += info.freq * cellArea / zoneArea / NUM_OFFSETS;
		if (roll < accum)
			return info.id;
	}
	return std::nullopt;
}

void StructureGenerator::drawPlacement(Chunk& chunk, const Placement& placement) const {
	ivec3 size = placement.blocks.size();
	ivec3 chunkOrigin = Converter::chunkToGlobal(chunk.getPosition());
	ivec2 chunkOrigin2D = Converter::to2D(chunkOrigin);
	ivec2 origin2D = Converter::to2D(placement.origin);
	ivec2 minPos = max(chunkOrigin2D, origin2D);
	ivec2 maxPos = min(chunkOrigin2D + Const::SECTION_SIDE, origin2D + Converter::to2D(size));
	for (int x = minPos.x; x < maxPos.x; ++x)
		for (int z = minPos.y; z < maxPos.y; ++z)
			for (int y = placement.origin.y; y < placement.origin.y + size.y; ++y) {
				ivec3 pos{ x, y, z };
				Block block = placement.blocks.at(pos - placement.origin);
				if (block.id != +BlockID::AIR)
					chunk.setBlock(pos - chunkOrigin, block);
			}
}

StructureTier StructureGenerator::tierOf(ivec3 size) {
	int footprint = std::max(size.x, size.z);
	if (footprint <= SMALL_MAX)
		return StructureTier::SMALL;
	if (footprint <= MEDIUM_MAX)
		return StructureTier::MEDIUM;
	return StructureTier::BIG;
}

void StructureGenerator::unloadFar(ivec2 centerChunk, int viewDistanceChunks) const {
	ivec2 centerXZ = centerChunk * Const::SECTION_SIDE;
	int keepRadius = (viewDistanceChunks + 3) * Const::SECTION_SIDE;
	for (const std::unique_ptr<StructureGrid>& grid : m_grids)
		grid->unloadFar(centerXZ, keepRadius);
}
