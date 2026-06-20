#include "ChunkMap.h"

#include "ResManager/ResManager.h"
#include "Util/DebugGL.h"
#include "Maths/Dir2D.h"
#include "Maths/Dir3D.h"
#include "Util/Array3D.h"
#include "Util/Logger.h"
#include "Maths/MiscMath.h"
#include "Maths/Converter.h"

#include <vector>
#include <algorithm>
#include <unordered_set>

const int ChunkMap::VIEW_DISTANCE{ 48 };
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
		if (chunkIt == m_chunks.end() || chunkIt->second->getState() <= Chunk::TO_LOAD_MESH) {
			loadBlocks(pos);
			for (Dir2D::Dir dir : Dir2D::all()) {
				loadBlocks(pos + Dir2D::to_ivec2(dir));
			}
			loadChunkMesh(pos);
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
	std::lock_guard<std::mutex> lock(m_chunksMutex);

	for (auto it = m_chunks.begin(); it != m_chunks.end();) {
		if (it->second->getState() == Chunk::TO_REMOVE) {
			it = m_chunks.erase(it);
			continue;
		} else if (!isInLoadDistance(it->second->getPosition())) {
			it->second->setState(Chunk::TO_RELEASE_MESH);
		}
		++it;
	}
}

void ChunkMap::update() {
	std::lock_guard<std::mutex> lock(m_chunksMutex);

	for (auto& c : m_chunks) {
		if (c.second->getState() == Chunk::TO_RELEASE_MESH) {
			c.second->releaseMesh();
		}
	}
	while (!m_chunksToUploadMesh.empty()) {
		Chunk& chunk = getChunk(m_chunksToUploadMesh.front());
		// Skip chunks already scheduled for unloading: uploading their VAOs now would leave GPU
		// resources to be freed on the loading thread when the chunk is erased.
		if (chunk.getState() == Chunk::TO_RENDER)
			chunk.uploadMesh();
		m_chunksToUploadMesh.pop();
	}

	while (!m_sectionsToReUploadMesh.empty()) {
		Section& section = getSection(m_sectionsToReUploadMesh.front());
		section.releaseMesh();
		section.uploadMesh();
		m_sectionsToReUploadMesh.pop();
	}
}

void ChunkMap::onChangeChunkState(Chunk& chunk, Chunk::State nextState) {
	if (chunk.getState() != Chunk::STATE_SIZE)
		--m_countChunks[chunk.getState()];
	if (nextState != Chunk::STATE_SIZE)
		++m_countChunks[nextState];
}

void ChunkMap::loadBlocks(ivec2 pos) {
	Chunk* chunk;
	{
		// Inserting into the map (possibly rehashing it) must be mutually exclusive
		// with any other thread accessing it, so hold the lock for this part only.
		std::lock_guard<std::mutex> lock(m_chunksMutex);
		auto it = m_chunks.find(pos);
		if (it == m_chunks.end()) // Chunk not in the ChunkMap
			it = m_chunks.emplace(pos, std::make_shared<Chunk>(this, pos)).first;
		chunk = it->second.get();
	}
	// Generating the blocks is the expensive part and only mutates the chunk itself,
	// not the shared map, so it runs without the lock.
	if (chunk->getState() == Chunk::TO_LOAD_BLOCKS) {
		chunk->loadBlocks();
	}
}

void ChunkMap::loadChunkMesh(ivec2 pos) {
	Chunk* chunk;
	{
		std::lock_guard<std::mutex> lock(m_chunksMutex);
		chunk = m_chunks.at(pos).get(); // Always present: loadBlocks(pos) ran first
	}
	if (chunk->getState() == Chunk::TO_LOAD_MESH) {
		chunk->loadMesh(); // Builds the mesh (reads neighbours); no map mutation
		// Publishing to the main thread, which uploads the VAOs in update(). Locking
		// here also ensures the generated mesh is visible to that thread.
		std::lock_guard<std::mutex> lock(m_chunksMutex);
		m_chunksToUploadMesh.push(pos);
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
	std::lock_guard<std::mutex> lock(m_chunksMutex);

	std::vector< std::tuple<int, Chunk* > > chunks;
	for (auto& chunkPos : m_chunks) {
		std::shared_ptr<Chunk>& chunk = chunkPos.second;
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
}

ivec2 ChunkMap::getCenter() {
	return m_center;
}

void ChunkMap::reloadSectionMesh(ivec3 pos) {
	std::lock_guard<std::mutex> lock(m_chunksMutex);

	auto chunkIt = m_chunks.find(Converter::to2D(pos));
	if (chunkIt != m_chunks.end() && chunkIt->second->getState() == Chunk::TO_RENDER &&
			0 <= pos.y && pos.y < chunkIt->second->getHeight()) {
		// In the extreme case where a chunk is edited near an unloaded chunk, neighboring chunks
		// could be unloaded.
		if (canChunkBeEdited(Converter::to2D(pos))) {
			chunkIt->second->getSection(pos.y).loadMesh();
			m_sectionsToReUploadMesh.push(pos);	
		}
	}
}

bool ChunkMap::canChunkBeEdited(ivec2 pos) {
	for (Dir2D::Dir dir : Dir2D::all()) {
		if (m_chunks.find(pos + Dir2D::to_ivec2(dir)) == m_chunks.end())
			return false;
	}
	return true;
}

void ChunkMap::reloadBlocksMeshes(const std::vector<ivec3>& blocks) {
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
		reloadSectionMesh(section);
	}
}

void ChunkMap::setCenter(ivec2 center) {
	m_center = center;
}

std::unordered_map<ivec2, std::shared_ptr<Chunk>, ChunkMap::Comp_ivec2, ChunkMap::Comp_ivec2>::const_iterator
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
	std::shared_ptr<Chunk> chunk;
	{
		// Only the lookup needs the lock. Copy the shared_ptr (not a raw pointer) so the chunk
		// survives even if the loading thread erases it while we edit.
		std::lock_guard<std::mutex> lock(m_chunksMutex);
		auto chunkIt = hasBlock(globalPos, true);
		if (chunkIt != m_chunks.end())
			chunk = chunkIt->second;
	}
	// The edit must happen without the lock held: Chunk::setBlock may extend the
	// chunk upwards and call back into reloadSectionMesh(), which locks m_chunksMutex
	// itself. Holding it here would self-deadlock.
	if (chunk != nullptr && chunk->getState() > Chunk::TO_LOAD_BLOCKS) {
		ivec2 innerChunkPos = Converter::globalToInnerChunk(globalPos);
		chunk->setBlock({ innerChunkPos.x, globalPos.y, innerChunkPos.y }, block);
	}
}

Block ChunkMap::getBlock(ivec3 globalPos) const {
	std::lock_guard<std::mutex> lock(m_chunksMutex);
	auto chunkIt = hasBlock(globalPos);
	if (chunkIt != m_chunks.end()) {
		ivec2 innerChunkPos = Converter::globalToInnerChunk(globalPos);
		return chunkIt->second->getBlock({ innerChunkPos.x, globalPos.y, innerChunkPos.y });
	}
	return { BlockID::AIR };
}

Chunk& ChunkMap::getChunk(ivec2 chunkPos) { // Must be a valid chunk position
	return *m_chunks.at(chunkPos).get();
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
	std::lock_guard<std::mutex> lock(m_chunksMutex);
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