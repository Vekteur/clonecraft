#pragma once

#include "Generator/Biome/BiomeID.h"
#include "Generator/Structure/StructureGrid.h"
#include "Generator/Structure/StructureID.h"

#include <memory>
#include <optional>
#include <vector>

class BiomeMap;
class Chunk;
class TerrainShaper;
struct ColumnInfo;

// Places structures (trees, cacti, ...) into chunks. Several grids of different cell sizes overlay the
// world; each grid resolves at most one structure per cell. Resolving a cell is a pure function of its
// position (with memoization), so every chunk that touches it agrees, regardless of visit order.
class StructureGenerator {
public:
	StructureGenerator(const BiomeMap& biomeMap, const TerrainShaper& terrain);

	// Draws every structure whose cell intersects the chunk.
	void place(Chunk& chunk) const;

	// Drops cached structure cells far from the player. Called by the orchestrator thread, like the
	// chunk unloading. viewDistanceChunks is the chunk view radius (margin is added on top).
	void unloadFar(ivec2 centerChunk, int viewDistanceChunks) const;

private:
	std::shared_ptr<const Placement> resolveCell(int gridIndex, ivec2 cell) const;
	std::shared_ptr<const Placement> computeCell(int gridIndex, ivec2 cell) const;
	// True if the candidate overlaps a structure in any strictly higher priority (earlier) grid. Only
	// those are checked, so lower priority structures always yield and the result stays order free.
	bool collidesWithHigher(int gridIndex, const Placement& candidate) const;
	std::optional<StructureID> pickStructure(BiomeID biome, StructureTier tier, double cellArea, double roll) const;
	void drawPlacement(Chunk& chunk, const Placement& placement) const;
	static StructureTier tierOf(ivec3 size);

	const BiomeMap& m_biomeMap;
	const TerrainShaper& m_terrain;

	// Ordered by collision priority: earlier grids win.
	std::vector<std::unique_ptr<StructureGrid>> m_grids;
};
