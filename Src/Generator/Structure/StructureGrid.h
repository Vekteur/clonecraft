#pragma once

#include "Block/Block.h"
#include "Generator/Structure/StructureID.h"
#include "Util/DynamicArray3D.h"
#include "Maths/GlmCommon.h"

#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <unordered_set>

enum class StructureTier { SMALL, MEDIUM, BIG };

// One resolved structure sitting in a grid cell: where it goes, the blocks built for it (kept so every
// chunk it touches reuses them instead of rebuilding), and the world columns it occupies for collision.
struct Placement {
	StructureID id;
	ivec3 origin;
	DynamicArray3D<Block> blocks;
	ivec2 footMin, footMax;
	std::unordered_set<ivec2, Comp_ivec2, Comp_ivec2> columns;

	Placement() : blocks(ivec3{ 0, 0, 0 }) {}
};

// A regular lattice of cells, each holding at most one structure. Several grids of different cell sizes
// and offsets overlay the world. A structure must fit inside its cell, so two structures in the same
// grid can never overlap. Resolved cells are memoized here and dropped once far from the player. The
// stored placement is a function of the cell position alone, so a racing double-resolve is harmless and
// dropping a cell is always safe. All access goes through the grid's own mutex.
class StructureGrid {
public:
	StructureGrid(int cellSize, ivec2 offset, StructureTier tier, int maxStructSize);

	int cellSize() const { return m_cellSize; }
	ivec2 offset() const { return m_offset; }
	StructureTier tier() const { return m_tier; }
	int maxStructSize() const { return m_maxStructSize; }

	ivec2 cellOf(ivec2 worldXZ) const;
	ivec2 cellMin(ivec2 cell) const;

	std::optional<std::shared_ptr<const Placement>> get(ivec2 cell) const;
	// Keeps the first result written for a cell and returns whichever one is now stored.
	std::shared_ptr<const Placement> store(ivec2 cell, std::shared_ptr<const Placement> placement) const;

	// Drop cells whose nearest point is further than keepRadius from centerXZ (Euclidean distance),
	// with one extra cell of margin so partly visible cells stay loaded.
	void unloadFar(ivec2 centerXZ, int keepRadius) const;

private:
	int m_cellSize;
	ivec2 m_offset;
	StructureTier m_tier;
	int m_maxStructSize;

	mutable std::mutex m_mutex;
	mutable std::unordered_map<ivec2, std::shared_ptr<const Placement>, Comp_ivec2, Comp_ivec2> m_cells;
};
