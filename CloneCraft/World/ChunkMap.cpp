#include "ChunkMap.h"

#include "ResManager/ResManager.h"
#include "Util/Debug.h"
#include "Maths/Dir2D.h"
#include "Maths/Dir3D.h"
#include "Util/Array3D.h"
#include "Util/Logger.h"
#include "Maths/MiscMath.h"
#include "Maths/Converter.h"

#include <GLFW\glfw3.h>
#include <vector>
#include <algorithm>
#include <unordered_set>

const int ChunkMap::VIEW_DISTANCE{ 20 };
const int ChunkMap::LOAD_DISTANCE{ VIEW_DISTANCE + 1 };
const int ChunkMap::SIDE{ (2 * VIEW_DISTANCE + 1) * Const::SECTION_SIDE };
const int ChunkMap::CHUNKS_PER_LOAD{ 8 };

ChunkMap::ChunkMap(ivec2 center) : m_center{ center } { }

ChunkMap::~ChunkMap() { }

void ChunkMap::load(const Frustum& frustum) {
	std::vector<std::tuple<bool, int, ivec2>> viewableChunks;
	for (int x = m_center.x - VIEW_DISTANCE; x <= m_center.x + VIEW_DISTANCE; ++x) {
		for (int y = m_center.y - VIEW_DISTANCE; y <= m_center.y + VIEW_DISTANCE; ++y) {
			ivec2 pos{ x, y };
			int dist = math::euclidianPow2(pos, m_center);
			if (dist <= VIEW_DISTANCE * VIEW_DISTANCE)
				viewableChunks.push_back({ !isChunkInFrustum(pos, frustum), dist, pos });
		}
	}
	std::sort(viewableChunks.begin(), viewableChunks.end(),
		[](const std::tuple<bool, int, ivec2>& c1, const std::tuple<bool, int, ivec2>& c2) {

		if (std::get<0>(c1) == std::get<0>(c2))
			return std::get<1>(c1) < std::get<1>(c2);
		return std::get<0>(c1) < std::get<0>(c2);
	});

	int loadedChunks = 0;
	for (auto& viewableChunk : viewableChunks) {
		ivec2 pos = std::get<2>(viewableChunk);
		auto chunkIt = m_chunks.find(pos);
		if (chunkIt == m_chunks.end() || chunkIt->second->getState() <= Chunk::TO_LOAD_FACES) {
			loadBlocks(pos);
			for (Dir2D::Dir dir : Dir2D::all()) {
				loadBlocks(pos + Dir2D::to_ivec2(dir));
			}
			loadFaces(pos);
			++loadedChunks;
			if (loadedChunks == CHUNKS_PER_LOAD)
				break;
		}
	}
}

bool ChunkMap::isInLoadDistance(ivec2 pos) const {
	return math::euclidianPow2(pos, m_center) <= LOAD_DISTANCE * LOAD_DISTANCE;
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
	while (!m_chunksToLoad.empty()) {
   		getChunk(m_chunksToLoad.front()).loadVAOs();
		m_chunksToLoad.pop();
	}

	while (!m_sectionsToReload.empty()) {
		getSection(m_sectionsToReload.front()).unloadVAOs();
		getSection(m_sectionsToReload.front()).loadVAOs();
		m_sectionsToReload.pop();
	}

	m_deleteChunksMutex.unlock();
}

void ChunkMap::onChangeChunkState(Chunk& chunk, Chunk::State nextState) {
	if (chunk.getState() != Chunk::STATE_SIZE)
		--m_countChunks[chunk.getState()];
	if (nextState != Chunk::STATE_SIZE)
		++m_countChunks[nextState];
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
		m_chunksToLoad.push(pos);
	}
}

bool ChunkMap::isChunkInFrustum(ivec2 chunkPos, const Frustum& frustum) {
	auto chunkIt = m_chunks.find(chunkPos);
	int height = ((chunkIt == m_chunks.end()) ? 
		Const::INIT_CHUNK_NB_SECTIONS : chunkIt->second->getHeight()) * Const::SECTION_HEIGHT;
	Box chunkBox = { { chunkPos.x * Const::SECTION_SIDE, 0.f, chunkPos.y * Const::SECTION_SIDE },
		{ Const::SECTION_SIDE, height, Const::SECTION_SIDE } };
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
	m_renderedChunks = chunks.size();
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
	m_deleteChunksMutex.lock();

	auto chunkIt = m_chunks.find(Converter::to2D(pos));
	if (chunkIt != m_chunks.end() && chunkIt->second->getState() == Chunk::TO_RENDER &&
			0 <= pos.y && pos.y < chunkIt->second->getHeight()) {
		chunkIt->second->getSection(pos.y).loadFaces();
		m_sectionsToReload.push(pos);
	}

	m_deleteChunksMutex.unlock();
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
			sectionsToUpdate.insert(Converter::globalToSection(block + Dir3D::to_ivec3(dir)));
		}
	}

	for (ivec3 section : sectionsToUpdate) {
		reloadSection(section);
	}
}

void ChunkMap::setCenter(ivec2 center) {
	m_center = center;
}

std::unordered_map<ivec2, std::unique_ptr<Chunk>, ChunkMap::Comp_ivec2, ChunkMap::Comp_ivec2>::const_iterator 
ChunkMap::hasBlock(ivec3 globalPos, bool canSurpass) const {
	auto chunkIt = m_chunks.find(Converter::globalToChunk(globalPos));
	if (chunkIt != m_chunks.end() && chunkIt->second->getState() > Chunk::TO_LOAD_BLOCKS &&
			0 <= globalPos.y &&
			(canSurpass || globalPos.y < chunkIt->second->getHeight() * Const::SECTION_HEIGHT))
		return chunkIt;
	else
		return m_chunks.end();
}

void ChunkMap::setBlock(ivec3 globalPos, Block block) {
	auto chunkIt = hasBlock(globalPos, true);
	if (chunkIt != m_chunks.end()) {
		ivec2 innerChunkPos = Converter::globalToInnerChunk(globalPos);
		chunkIt->second->setBlock({ innerChunkPos.x, globalPos.y, innerChunkPos.y }, block);
	}
}

Block ChunkMap::getBlock(ivec3 globalPos) const {
	auto chunkIt = hasBlock(globalPos);
	if (chunkIt != m_chunks.end()) {
		ivec2 innerChunkPos = Converter::globalToInnerChunk(globalPos);
		return chunkIt->second->getBlock({ innerChunkPos.x, globalPos.y, innerChunkPos.y });
	}
	return { BlockID::AIR };
}

Chunk& ChunkMap::getChunk(ivec2 chunkPos) { // Must be a valid chunk position
	return *m_chunks[chunkPos].get();
}

const Chunk& ChunkMap::getChunk(ivec2 chunkPos) const { // Must be a valid chunk position
	return *m_chunks.at(chunkPos).get();
}

Section& ChunkMap::getSection(ivec3 sectionPos) { // Must be a valid section position
	return getChunk(Converter::to2D(sectionPos)).getSection(sectionPos.y);
}

const Section& ChunkMap::getSection(ivec3 sectionPos) const { // Must be a valid section position
	return getChunk(Converter::to2D(sectionPos)).getSection(sectionPos.y);
}

int ChunkMap::size() {
	return m_chunks.size();
}

void ChunkMap::stop() {
	m_mustStop = true;
}

int ChunkMap::chunksAtLeastInState(Chunk::State minState) {
	int sum = 0;
	for (int state = minState; state < Chunk::STATE_SIZE; ++state) {
		sum += m_countChunks[state];
	}
	return sum;
}

int ChunkMap::chunksInState(Chunk::State state) {
	return m_countChunks[state];
}

int ChunkMap::getRenderedChunks() {
	return m_renderedChunks;
}