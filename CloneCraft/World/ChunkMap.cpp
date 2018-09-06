#include "ChunkMap.h"

#include "ResManager.h"
#include "Debug.h"
#include "Dir2D.h"
#include "Dir3D.h"
#include "Array3D.h"
#include "Logger.h"
#include "MiscMath.h"

#include <GLFW\glfw3.h>
#include <vector>
#include <algorithm>
#include <unordered_set>

ChunkMap::ChunkMap(ivec2 center) : m_center{ center } { }

ChunkMap::~ChunkMap() { }

void ChunkMap::load(const Frustum& frustum) {
	std::vector<std::tuple<bool, int, ivec2>> viewableChunks;
	for (int x = m_center.x - VIEW_DISTANCE; x <= m_center.x + VIEW_DISTANCE; ++x) {
		for (int y = m_center.y - VIEW_DISTANCE; y <= m_center.y + VIEW_DISTANCE; ++y) {
			ivec2 pos{ x, y };
			int dist = math::euclidianPow2(pos, m_center);
			if (dist <= LOAD_DISTANCE * LOAD_DISTANCE) // Take a small margin
				viewableChunks.push_back({ !isChunkInFrustum(pos, frustum), dist, pos });
		}
	}
	std::sort(viewableChunks.begin(), viewableChunks.end(),
		[](const std::tuple<bool, int, ivec2>& c1, const std::tuple<bool, int, ivec2>& c2) {

		if (std::get<0>(c1) == std::get<0>(c2))
			return std::get<1>(c1) <= std::get<1>(c2);
		return std::get<0>(c1) < std::get<0>(c2);
	});

	int loadedChunks = 0;
	for (auto& viewableChunk : viewableChunks) {
		ivec2 pos = std::get<2>(viewableChunk);
		auto chunkIt = m_chunks.find(pos);
		if (chunkIt == m_chunks.end() || chunkIt->second->getState() <= Chunk::TO_LOAD_FACES) {
			loadBlocks(pos);
			for (ivec2 dir : Dir2D::all_dirs()) {
				loadBlocks(pos + dir);
			}
			loadFaces(pos);
			++loadedChunks;
			if (loadedChunks == CHUNKS_PER_LOAD)
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
		} else if (!isInLoadDistance(it->second->getPosition())) {
			it->second->setState(Chunk::TO_UNLOAD_VAOS);
		}
		++it;
	}
	m_deleteChunksMutex.unlock();
}

void ChunkMap::update() {
	m_deleteChunksMutex.lock();
	for (auto& c : m_chunks) {
		if (c.second->getState() == Chunk::TO_UNLOAD_VAOS) {
			c.second->unloadVAOs();
		}
	}
	while (!toLoadChunks.empty()) {
		getChunk(toLoadChunks.front()).loadVAOs();
		toLoadChunks.pop();
	}

	while (!toReloadSections.empty()) {
		getSection(toReloadSections.front()).unloadVAOs();
		getSection(toReloadSections.front()).loadVAOs();
		toReloadSections.pop();
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
		m_chunks[pos]->loadBlocks();
	}
}

void ChunkMap::loadFaces(ivec2 pos) {
	if (m_chunks[pos]->getState() == Chunk::TO_LOAD_FACES) {
		m_chunks[pos]->loadFaces();
		toLoadChunks.push(pos);
	}
}

bool ChunkMap::isChunkInFrustum(ivec2 chunkPos, const Frustum& frustum) {
	Box chunkBox = { { chunkPos.x * Const::CHUNK_SIDE, 0.f, chunkPos.y * Const::CHUNK_SIDE },
		{ Const::CHUNK_SIDE, Const::CHUNK_HEIGHT, Const::CHUNK_SIDE } };
	return !frustum.isBoxOutside(chunkBox);
}

void ChunkMap::render(const Frustum& frustum, const DefaultRenderer* defaultRenderer, const WaterRenderer* waterRenderer) {
	m_deleteChunksMutex.lock();

	std::vector< std::tuple<int, Chunk* > > chunks;
	for (auto& chunkPos : m_chunks) {
		std::unique_ptr<Chunk>& chunk = chunkPos.second;
		if (chunk->getState() == Chunk::TO_RENDER && isChunkInFrustum(chunk->getPosition(), frustum)) {
			chunks.push_back({ math::manhattan(chunk->getPosition(), m_center), chunk.get() });
		}
	}
	renderedChunks = chunks.size();
	std::sort(chunks.begin(), chunks.end(), [](const std::tuple<int, Chunk* >& c1, const std::tuple<int, Chunk* >& c2) {
		return std::get<0>(c1) < std::get<0>(c2);
	});

	if (defaultRenderer != nullptr) {
		for (auto& chunk : chunks) {
			std::get<1>(chunk)->render(*defaultRenderer);
		}
	}
	if (waterRenderer != nullptr) {
		for (auto& chunk : chunks) {
			std::get<1>(chunk)->render(*waterRenderer);
		}
	}

	m_deleteChunksMutex.unlock();
}

ivec2 ChunkMap::getCenter() {
	return m_center;
}

void ChunkMap::reloadSection(ivec3 pos) {
	auto chunkIt = m_chunks.find({ pos.x, pos.z });
	if (chunkIt != m_chunks.end() && chunkIt->second->getState() >= Chunk::TO_RENDER &&
		0 <= pos.y && pos.y < Const::CHUNK_NB_SECTIONS) {
		chunkIt->second->getSection(pos.y).loadFaces();
		toReloadSections.push(pos);
	}
}

void ChunkMap::reloadBlocks(const std::vector<ivec3>& blocks) {
	struct Comp_ivec3 {
		size_t operator()(ivec3 vec) const {
			return std::hash<int>()(vec.x) ^ (std::hash<int>()(vec.y) << 1) ^ (std::hash<int>()(vec.z) << 2);
		}
		bool operator()(ivec3 a, ivec3 b) const {
			return a.x == b.x && a.y == b.y && a.z == b.z;
		}
	};

	std::unordered_set<ivec3, Comp_ivec3> sectionsToUpdate;
	for (ivec3 block : blocks) {
		sectionsToUpdate.insert(Converter::globalToSection(block));
		for (Dir3D::Dir dir : Dir3D::all()) {
			sectionsToUpdate.insert(Converter::globalToSection(block + Dir3D::find(dir)));
		}
	}

	for (ivec3 section : sectionsToUpdate) {
		reloadSection(section);
	}
}

void ChunkMap::setCenter(ivec2 center) {
	m_center = center;
}

std::unordered_map<ivec2, std::unique_ptr<Chunk>, ChunkMap::Comp_ivec2, ChunkMap::Comp_ivec2>::iterator 
ChunkMap::hasBlock(ivec3 globalPos) {
	auto chunkIt = m_chunks.find(Converter::globalToChunk(globalPos));
	if (chunkIt != m_chunks.end() && chunkIt->second->getState() > Chunk::TO_LOAD_BLOCKS &&
		0 <= globalPos.y && globalPos.y < Const::CHUNK_HEIGHT)
		return chunkIt;
	else
		return m_chunks.end();
}

bool ChunkMap::isInLoadDistance(ivec2 pos) {
	return m_center.x - LOAD_DISTANCE <= pos.x && pos.x <= m_center.x + LOAD_DISTANCE &&
		m_center.y - LOAD_DISTANCE <= pos.y && pos.y <= m_center.y + LOAD_DISTANCE;
}

void ChunkMap::setBlock(ivec3 globalPos, Block block) {
	auto chunkIt = hasBlock(globalPos);
	if (chunkIt != m_chunks.end()) {
		chunkIt->second->getSection(floorDiv(globalPos.y, Const::SECTION_HEIGHT))
			.setBlock(Converter::globalToInnerSection(globalPos), block);
	}
}

Block ChunkMap::getBlock(ivec3 globalPos) {
	auto chunkIt = hasBlock(globalPos);
	if (chunkIt != m_chunks.end()) {
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

