#include "ChunkMap.h"

#include "ResManager.h"
#include "Debug.h"
#include "Dir2D.h"
#include "Array3D.h"
#include "Logger.h"
#include "MiscMath.h"

#include <GLFW\glfw3.h>
#include <vector>
#include <algorithm>

ChunkMap::ChunkMap(ivec2 center) : m_center{ center } {
}

ChunkMap::~ChunkMap() {
}

void ChunkMap::load() {
	loadBlocks(ivec2{ m_center.x, m_center.y }); // load the blocks of the center chunk
	for (int d = 1; d <= LOAD_DISTANCE; ++d) {
		// load the blocks in the chunks at distance d
		for (int dir = 0; dir < Dir2D::SIZE; ++dir) {
			ivec2 curr = Dir2D::find(static_cast<Dir2D::Dir>(dir));
			ivec2 prev = Dir2D::prev(static_cast<Dir2D::Dir>(dir));
			ivec2 next = Dir2D::next(static_cast<Dir2D::Dir>(dir));
			for (ivec2 relPos = prev * d; relPos != next * d; relPos += next) {
				ivec2 pos = relPos + m_center + curr * d;
				loadBlocks(pos);
				if (mustStop)
					return;
			}
		}

		// load the faces in the chunks at distance d - 1
		int c = d - 1;
		if (c == 0) {
			loadFaces(ivec2{ m_center.x, m_center.y });
		} else {
			for (int dir = 0; dir < Dir2D::SIZE; ++dir) {
				ivec2 curr = Dir2D::find(static_cast<Dir2D::Dir>(dir));
				ivec2 prev = Dir2D::prev(static_cast<Dir2D::Dir>(dir));
				ivec2 next = Dir2D::next(static_cast<Dir2D::Dir>(dir));
				for (ivec2 relPos = prev * c; relPos != next * c; relPos += next) {
					ivec2 pos = relPos + m_center + curr * c;
					loadFaces(pos);
					if (mustStop)
						return;
				}
			}
		}

		glFlush();

		if (m_newCenter != m_center) {
			m_center = m_newCenter;
			break;
		}
	}
}

void ChunkMap::unloadFarChunks() {
	m_deleteChunksMutex.lock();
	for (auto it = m_chunks.begin(); it != m_chunks.end();) {
		if (it->second->getState() == Chunk::TO_REMOVE) {
			it = m_chunks.erase(it);
			continue;
		} else if (!isInChunkMap(it->second->getPosition())) {
			it->second->setState(Chunk::TO_UNLOAD_VAOS);
		}
		++it;
	}
	m_deleteChunksMutex.unlock();
}

void ChunkMap::onChangeChunkState(Chunk& chunk, Chunk::State nextState) {
	if (chunk.getState() != Chunk::STATE_SIZE)
		--countChunks[chunk.getState()];
	if (nextState != Chunk::STATE_SIZE)
		++countChunks[nextState];
}

void ChunkMap::loadBlocks(ivec2 pos) {
	if (m_chunks.find(pos) == m_chunks.end()) { // Chunk not in the ChunkMap
		m_chunks.emplace(pos, std::make_unique<Chunk>(this, pos));
	}
	if (m_chunks[pos]->getState() == Chunk::TO_LOAD_BLOCKS) {
		m_chunks[pos]->loadBlocks(); // Load the blocks
	}
}

void ChunkMap::loadFaces(ivec2 pos) {
	if (m_chunks[pos]->getState() == Chunk::TO_LOAD_FACES) {
		m_chunks[pos]->loadFaces(); // Load the faces
	}
}

void ChunkMap::update() {
	m_deleteChunksMutex.lock();
	for (auto& c : m_chunks) {
		Chunk::State state = c.second->getState();
		if (state == Chunk::TO_LOAD_VAOS) {
			c.second->loadVAOs();
		} else if (state == Chunk::TO_UNLOAD_VAOS) {
			c.second->unloadVAOs();
		}
	}
	m_deleteChunksMutex.unlock();
}

void ChunkMap::render(const Frustum& frustum, DefaultRenderer &defaultRenderer, WaterRenderer& waterRenderer) {
	m_deleteChunksMutex.lock();

	std::vector< std::tuple<int, Chunk* > > chunks;
	for (auto& posChunk : m_chunks) {
		std::unique_ptr<Chunk>& chunk = posChunk.second;
		Box chunkBox = { {chunk->getPosition().x * Const::CHUNK_SIDE, 0.f, chunk->getPosition().y * Const::CHUNK_SIDE }, 
		{ Const::CHUNK_SIDE, Const::CHUNK_HEIGHT, Const::CHUNK_SIDE } };
		if (chunk->getState() == Chunk::TO_RENDER && !frustum.isBoxOutside(chunkBox)) {
			chunks.push_back({ math::manhattan(chunk->getPosition(), m_center), chunk.get() });
		}
	}
	renderedChunks = chunks.size();
	std::sort(chunks.begin(), chunks.end(), [](const std::tuple<int, Chunk* >& c1, const std::tuple<int, Chunk* >& c2) {
		return std::get<0>(c1) < std::get<0>(c2);
	});

	for (auto& chunk : chunks) {
		std::get<1>(chunk)->render(defaultRenderer, waterRenderer);
	}

	m_deleteChunksMutex.unlock();
}

void ChunkMap::setCenter(ivec2 center) {
	m_newCenter = center;
}

bool ChunkMap::isInChunkMap(ivec2 pos) {
	return m_center.x - LOAD_DISTANCE <= pos.x && pos.x <= m_center.x + LOAD_DISTANCE &&
		m_center.y - LOAD_DISTANCE <= pos.y && pos.y <= m_center.y + LOAD_DISTANCE;
}

ivec2 ChunkMap::getCenter() {
	return m_center;
}

void ChunkMap::setBlock(ivec3 globalPos, Block block) {
	m_chunks[Converter::globalToChunk(globalPos)]->getSection(floorDiv(globalPos.y, Const::SECTION_HEIGHT))
		.setBlock(Converter::globalToInnerSection(globalPos), block);
}

Block ChunkMap::getBlock(ivec3 globalPos) {
	auto chunkIt = m_chunks.find(Converter::globalToChunk(globalPos));
	if (chunkIt != m_chunks.end() && chunkIt->second->getState() > Chunk::TO_LOAD_BLOCKS && 
			0 <= globalPos.y && globalPos.y < Const::CHUNK_HEIGHT) {
		return chunkIt->second->getSection(floorDiv(globalPos.y, Const::SECTION_HEIGHT))
			.getBlock(Converter::globalToInnerSection(globalPos));
	}
	return { ID::AIR };
}

Chunk& ChunkMap::getChunk(ivec2 chunkPos) { // Must be a valid chunk position
	return *m_chunks[chunkPos].get();
}

Section& ChunkMap::getSection(ivec3 sectionPos) { // Must be a valid section position
	return getChunk({ sectionPos.x, sectionPos.z }).getSection(sectionPos.y);
}

int ChunkMap::size() {
	return m_chunks.size();
}

void ChunkMap::stop() {
	mustStop = true;
}

int ChunkMap::chunksAtLeastInState(Chunk::State minState) {
	int sum = 0;
	for (int state = minState; state < Chunk::STATE_SIZE; ++state) {
		sum += countChunks[state];
	}
	return sum;
}

int ChunkMap::chunksInState(Chunk::State state) {
	return countChunks[state];
}

int ChunkMap::getRenderedChunks() {
	return renderedChunks;
}

