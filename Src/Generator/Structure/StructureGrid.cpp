#include "StructureGrid.h"

#include "Maths/Converter.h"

StructureGrid::StructureGrid(int cellSize, ivec2 offset, StructureTier tier, int maxStructSize)
	: m_cellSize{ cellSize }, m_offset{ offset }, m_tier{ tier }, m_maxStructSize{ maxStructSize } {}

ivec2 StructureGrid::cellOf(ivec2 worldXZ) const {
	return floorDiv(worldXZ - m_offset, ivec2(m_cellSize));
}

ivec2 StructureGrid::cellMin(ivec2 cell) const {
	return cell * m_cellSize + m_offset;
}

std::optional<std::shared_ptr<const Placement>> StructureGrid::get(ivec2 cell) const {
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_cells.find(cell);
	if (it == m_cells.end())
		return std::nullopt;
	return it->second;
}

std::shared_ptr<const Placement> StructureGrid::store(ivec2 cell, std::shared_ptr<const Placement> placement) const {
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_cells.try_emplace(cell, std::move(placement)).first->second;
}

void StructureGrid::unloadFar(ivec2 centerXZ, int keepRadius) const {
	std::lock_guard<std::mutex> lock(m_mutex);
	long long keepPow2 = static_cast<long long>(keepRadius) * keepRadius;
	for (auto it = m_cells.begin(); it != m_cells.end();) {
		// Keep the cell if any part of it is within keepRadius, so a cell that is only partly in view
		// is not dropped. Measure the Euclidean distance from the player to the nearest point of the
		// cell, then grant one extra cell of margin to avoid churn at the boundary.
		ivec2 boxMin = cellMin(it->first);
		ivec2 boxMax = boxMin + (m_cellSize - 1);
		ivec2 d = glm::max(ivec2{ 0, 0 }, glm::max(boxMin - centerXZ, centerXZ - boxMax) - ivec2{ m_cellSize });
		if (static_cast<long long>(d.x) * d.x + static_cast<long long>(d.y) * d.y > keepPow2)
			it = m_cells.erase(it);
		else
			++it;
	}
}
