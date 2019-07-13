#include "Structure.h"

#include "Maths/Converter.h"
#include "Maths/Dir2D.h"

Structure::Structure(ivec3 size) : m_blocks{ size } {}

Block Structure::getBlock(ivec3 pos) const {
	return m_blocks.at(pos);
}

ivec3 Structure::size() const {
	return m_blocks.size();
}

ivec2 Structure::getCenterPos(ivec2 globalPos) const {
	return globalPos + Converter::to2D(size()) / 2;
}

void Structure::add(ivec3 pos, Block block) {
	m_blocks.at(pos) = block;
}

void Structure::addSymetrically(ivec3 pos, Block block) {
	ivec2 center = getCenterPos();
	ivec2 relToCenter = Converter::to2D(pos) - center;
	for (int i = 0; i < 4; ++i) {
		ivec2 rotatedPos = relToCenter + center;
		m_blocks.at({ rotatedPos.x, pos.y, rotatedPos.y }) = block;
		relToCenter = { relToCenter.y, -relToCenter.x };
	}
}

void Structure::fill(ivec3 low, ivec3 high, Block block) {
	for (int y = low.y; y <= high.y; ++y)
		for (int x = low.x; x <= high.x; ++x)
			for (int z = low.z; z <= high.z; ++z) {
				add({ x, y, z }, block);
			}
}

void Structure::fillSymetrically(ivec3 low, ivec3 high, Block block) {
	for (int y = low.y; y <= high.y; ++y)
		for (int x = low.x; x <= high.x; ++x)
			for (int z = low.z; z <= high.z; ++z) {
				addSymetrically({ x, y, z }, block);
			}
}