#include "ChunkMap.h"
#include "ResManager.h"
#include "Debug.h"
#include "Dir2D.h"

#include "Array3D.h"

#include <GLFW\glfw3.h>

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

void ChunkMap::loadBlocks(ivec2 pos) {
	if (m_chunks.find(pos) == m_chunks.end()) // Chunk not in the ChunkMap
		m_chunks.emplace(pos, std::make_unique<Chunk>(this, pos));
	if (m_chunks[pos]->getState() == Chunk::TO_LOAD_BLOCKS)
		m_chunks[pos]->loadBlocks(); // Load the blocks
}

void ChunkMap::loadFaces(ivec2 pos) {
	if (m_chunks.find(pos) == m_chunks.end()) // Chunk not in the ChunkMap
		m_chunks.emplace(pos, std::make_unique<Chunk>(this, pos));
	if (m_chunks[pos]->getState() == Chunk::TO_LOAD_FACES)
		m_chunks[pos]->loadFaces(); // Load the faces
}

void ChunkMap::update() {
	m_deleteChunksMutex.lock();
	for (auto& c : m_chunks) {
		Chunk::State state = c.second->getState();
		if (state == Chunk::TO_LOAD_VAOS)
			c.second->loadVAOs();
		else if (state == Chunk::TO_UNLOAD_VAOS)
			c.second->unloadVAOs();
	}
	m_deleteChunksMutex.unlock();
}

void ChunkMap::render() {
	m_deleteChunksMutex.lock();

	for (auto& c : m_chunks)
		if (c.second->getState() == Chunk::TO_RENDER)
			c.second->render(ResManager::getShader("cube"), ResManager::getTexture("stone"));

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

Chunk& ChunkMap::getChunk(ivec2 pos) {
	return *m_chunks[pos].get();
}

Section& ChunkMap::getSection(ivec3 pos) {
	ivec2 chunkPos{ pos.x, pos.z };
	return getChunk(chunkPos).getSection(pos.y);
}

int ChunkMap::getSize() {
	return m_chunks.size();
}