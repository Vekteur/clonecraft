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
#include <thread>
#include <chrono>

#ifndef CC_VIEW_DISTANCE
#define CC_VIEW_DISTANCE 32
#endif
const int ChunkMap::VIEW_DISTANCE{ CC_VIEW_DISTANCE };
const int ChunkMap::LOAD_DISTANCE{ VIEW_DISTANCE + 1 };
const int ChunkMap::SIDE{ (2 * VIEW_DISTANCE + 1) * Const::SECTION_SIDE };
#ifndef CC_LOADING_WORKERS_COUNT
#define CC_LOADING_WORKERS_COUNT 4
#endif
const int ChunkMap::LOADING_WORKERS_COUNT{ CC_LOADING_WORKERS_COUNT };

ChunkMap::ChunkMap(ivec2 center) : m_center{ center } { }

ChunkMap::~ChunkMap() { }

void ChunkMap::update() {
	std::lock_guard<std::mutex> lock(m_chunksMutex);

	for (auto& c : m_chunks) {
		if (c.second->getState() == Chunk::TO_RELEASE_MESH) {
			c.second->releaseMesh();
		}
	}
	while (!m_chunksToUploadMesh.empty()) {
		ivec2 pos = m_chunksToUploadMesh.front();
		Chunk& chunk = getChunk(pos);
		// Skip chunks already scheduled for unloading: uploading their VAOs now would leave GPU
		// resources to be freed on the loading thread when the chunk is erased.
		if (chunk.getState() == Chunk::TO_RENDER)
			chunk.uploadMesh();
		m_chunksPendingUpload.erase(pos);
		m_chunksToUploadMesh.pop();
	}

	while (!m_sectionsToReUploadMesh.empty()) {
		Section& section = getSection(m_sectionsToReUploadMesh.front());
		section.releaseMesh();
		section.uploadMesh();
		m_sectionsToReUploadMesh.pop();
	}
}

void ChunkMap::render(const Frustum& frustum, const DefaultRenderer* defaultRenderer, const WaterRenderer* waterRenderer) {
	std::lock_guard<std::mutex> lock(m_chunksMutex);

	std::vector< std::tuple<int, Chunk* > > chunks;
	for (auto& chunkPos : m_chunks) {
		std::shared_ptr<Chunk>& chunk = chunkPos.second;
		if (chunk->getState() == Chunk::TO_RENDER && isChunkInFrustum(chunk.get(), frustum)) {
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
	if (chunk != nullptr && chunk->getState() >= Chunk::TO_LOAD_MESH) {
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

void ChunkMap::setCenter(ivec2 center) {
	std::lock_guard<std::mutex> lock(m_chunksMutex);
	m_center = center;
}

ivec2 ChunkMap::getCenter() {
	std::lock_guard<std::mutex> lock(m_chunksMutex);
	return m_center;
}

void ChunkMap::stop() {
	m_mustStop = true;
}

int ChunkMap::size() {
	std::lock_guard<std::mutex> lock(m_chunksMutex);
	return m_chunks.size();
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

void ChunkMap::refreshSelection(const Frustum& frustum) {
	ivec2 center;
	{
		std::lock_guard<std::mutex> lock(m_chunksMutex);
		center = m_center;
	}
	// Built without the lock: just positions, ordered by frustum (visible first) then distance.
	// The frustum test uses a fixed height so it never touches the map.
	std::vector<std::tuple<bool, int, ivec2>> viewableChunks;
	for (int x = center.x - VIEW_DISTANCE; x <= center.x + VIEW_DISTANCE; ++x) {
		for (int y = center.y - VIEW_DISTANCE; y <= center.y + VIEW_DISTANCE; ++y) {
			ivec2 pos{ x, y };
			int dist = math::euclidianPow2(pos, center);
			if (dist <= VIEW_DISTANCE * VIEW_DISTANCE)
				viewableChunks.push_back({ !isChunkInFrustumApprox(pos, frustum), dist, pos });
		}
	}
	std::sort(viewableChunks.begin(), viewableChunks.end(),
		[](const std::tuple<bool, int, ivec2>& c1, const std::tuple<bool, int, ivec2>& c2) {

		if (std::get<0>(c1) == std::get<0>(c2))
			return std::get<1>(c1) < std::get<1>(c2);
		return std::get<0>(c1) < std::get<0>(c2);
	});

	std::vector<ivec2> selection;
	selection.reserve(viewableChunks.size());
	for (auto& viewableChunk : viewableChunks)
		selection.push_back(std::get<2>(viewableChunk));

	std::lock_guard<std::mutex> lock(m_chunksMutex);
	m_toSelect = std::move(selection);
	m_selectCursor = 0;
}

void ChunkMap::processNextTask() {
	enum { BLOCK, MESH, SELECT, NONE } type = NONE;
	ivec2 pos;
	{
		std::lock_guard<std::mutex> lock(m_chunksMutex);
		if (!m_toLoadBlocks.empty()) {
			pos = m_toLoadBlocks.front(); m_toLoadBlocks.pop(); m_inLoadBlocks.erase(pos); type = BLOCK;
		} else if (!m_toLoadMeshes.empty()) {
			pos = m_toLoadMeshes.front(); m_toLoadMeshes.pop(); m_inLoadMeshes.erase(pos); type = MESH;
		} else if (m_selectCursor < m_toSelect.size()) {
			pos = m_toSelect[m_selectCursor++]; type = SELECT;
		}
	}
	switch (type) {
		case BLOCK:  doLoadBlocks(pos); break;
		case MESH:   doLoadMesh(pos);   break;
		case SELECT: doSelectChunk(pos); break;
		case NONE:   std::this_thread::sleep_for(std::chrono::milliseconds(1)); break;
	}
}

void ChunkMap::doSelectChunk(ivec2 pos) {
	// Queue blocks for this chunk and its neighbors (a chunk's mesh needs its neighbors' blocks).
	std::lock_guard<std::mutex> lock(m_chunksMutex);
	enqueueBlocksIfNeeded(pos);
	for (Dir2D::Dir dir : Dir2D::all())
		enqueueBlocksIfNeeded(pos + Dir2D::to_ivec2(dir));
}

void ChunkMap::enqueueBlocksIfNeeded(ivec2 pos) {
	auto it = m_chunks.find(pos);
	bool needsBlocks = (it == m_chunks.end()) || (it->second->getState() == Chunk::TO_LOAD_BLOCKS);
	if (needsBlocks && m_inLoadBlocks.insert(pos).second)
		m_toLoadBlocks.push(pos);
}

void ChunkMap::unloadFarChunks() {
	std::lock_guard<std::mutex> lock(m_chunksMutex);

	for (auto it = m_chunks.begin(); it != m_chunks.end();) {
		Chunk& chunk = *it->second;
		if (chunk.getState() == Chunk::TO_REMOVE) {
			it = m_chunks.erase(it);
			continue;
		} else if (!isInLoadDistance(chunk.getPosition())) {
			// CAS from a settled state so we never overwrite a worker's in-flight LOADING_* claim;
			// such a chunk is caught on the next pass once the worker is done with it.
			chunk.casState(Chunk::TO_LOAD_BLOCKS, Chunk::TO_RELEASE_MESH) ||
			chunk.casState(Chunk::TO_LOAD_MESH, Chunk::TO_RELEASE_MESH) ||
			chunk.casState(Chunk::TO_RENDER, Chunk::TO_RELEASE_MESH);
		}
		++it;
	}
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

bool ChunkMap::canChunkBeEdited(ivec2 pos) {
	for (Dir2D::Dir dir : Dir2D::all()) {
		if (m_chunks.find(pos + Dir2D::to_ivec2(dir)) == m_chunks.end())
			return false;
	}
	return true;
}

void ChunkMap::reloadSectionMesh(ivec3 pos) {
	std::lock_guard<std::mutex> lock(m_chunksMutex);

	ivec2 pos2D = Converter::to2D(pos);
	auto chunkIt = m_chunks.find(pos2D);
	if (chunkIt != m_chunks.end() && chunkIt->second->getState() == Chunk::TO_RENDER &&
			0 <= pos.y && pos.y < chunkIt->second->getHeight()) {
		// In the extreme case where a chunk is edited on the border of the map, neighboring chunks
		// could be unloaded.
		if (canChunkBeEdited(pos2D)) {
			NeighborChunks neighbors{};
			for (Dir2D::Dir dir : Dir2D::all()) {
				auto it = m_chunks.find(pos2D + Dir2D::to_ivec2(dir));
				neighbors[dir] = (it != m_chunks.end()) ? it->second.get() : nullptr;
			}
			chunkIt->second->getSection(pos.y).loadMesh(neighbors);
			// If the chunk still owes a full upload, that upload already carries this freshly rebuilt
			// section; a section upload would run after it and push an already-consumed empty mesh.
			if (m_chunksPendingUpload.count(pos2D) == 0)
				m_sectionsToReUploadMesh.push(pos);
		}
	}
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

void ChunkMap::onChangeChunkState(Chunk::State previous, Chunk::State next) {
	if (previous != Chunk::STATE_SIZE)
		--m_countChunks[previous];
	if (next != Chunk::STATE_SIZE)
		++m_countChunks[next];
}

std::unordered_map<ivec2, std::shared_ptr<Chunk>, ChunkMap::Comp_ivec2, ChunkMap::Comp_ivec2>::const_iterator
ChunkMap::hasBlock(ivec3 globalPos, bool canSurpass) const {
	auto chunkIt = m_chunks.find(Converter::globalToChunk(globalPos));
	if (chunkIt != m_chunks.end() && chunkIt->second->getState() >= Chunk::TO_LOAD_MESH &&
			0 <= globalPos.y &&
			(canSurpass || globalPos.y < chunkIt->second->getHeight() * Const::SECTION_HEIGHT))
		return chunkIt;
	else
		return m_chunks.end();
}

bool ChunkMap::isInLoadDistance(ivec2 pos) const {
	return math::euclidianPow2(pos, m_center) <= LOAD_DISTANCE * LOAD_DISTANCE;
}

void ChunkMap::doLoadBlocks(ivec2 pos) {
	std::shared_ptr<Chunk> chunk;
	{
		std::lock_guard<std::mutex> lock(m_chunksMutex);
		if (!isInLoadDistance(pos)) // moved out of range while queued
			return;
		auto it = m_chunks.find(pos);
		if (it == m_chunks.end()) // Chunk not in the ChunkMap
			it = m_chunks.emplace(pos, std::make_shared<Chunk>(this, pos)).first;
		chunk = it->second; // keep alive while we generate, even if it gets erased
	}
	// Claim the chunk; if another worker beat us (or it is already loaded) there is nothing to do.
	if (!chunk->casState(Chunk::TO_LOAD_BLOCKS, Chunk::LOADING_BLOCKS))
		return;
	// Generating only mutates this chunk, so it runs without the lock. loadBlocks() ends at
	// TO_LOAD_MESH.
	chunk->loadBlocks();
	// This chunk's blocks just appeared, which may complete the mesh prerequisites of itself and
	// of each neighbor it borders, so re-check them all.
	std::lock_guard<std::mutex> lock(m_chunksMutex);
	enqueueMeshIfReady(pos);
	for (Dir2D::Dir dir : Dir2D::all())
		enqueueMeshIfReady(pos + Dir2D::to_ivec2(dir));
}

void ChunkMap::enqueueMeshIfReady(ivec2 pos) {
	auto it = m_chunks.find(pos);
	if (it == m_chunks.end() || it->second->getState() != Chunk::TO_LOAD_MESH)
		return; // missing, still loading blocks, or already meshing/meshed
	for (Dir2D::Dir dir : Dir2D::all()) {
		auto neighborIt = m_chunks.find(pos + Dir2D::to_ivec2(dir));
		if (neighborIt == m_chunks.end() || neighborIt->second->getState() < Chunk::TO_LOAD_MESH)
			return; // a neighbor's blocks are not ready, so this mesh is not either
	}
	if (m_inLoadMeshes.insert(pos).second)
		m_toLoadMeshes.push(pos);
}

void ChunkMap::doLoadMesh(ivec2 pos) {
	std::shared_ptr<Chunk> chunk;
	std::array<std::shared_ptr<Chunk>, Dir2D::SIZE> keepAlive; // keep neighbors alive during the build
	NeighborChunks neighbors{};
	{
		std::lock_guard<std::mutex> lock(m_chunksMutex);
		auto it = m_chunks.find(pos);
		if (it == m_chunks.end())
			return;
		// Snapshot the neighboring chunks so the build below never reads the map.
		for (Dir2D::Dir dir : Dir2D::all()) {
			auto neighborIt = m_chunks.find(pos + Dir2D::to_ivec2(dir));
			if (neighborIt != m_chunks.end()) {
				keepAlive[dir] = neighborIt->second;
				neighbors[dir] = neighborIt->second.get();
			}
		}
		if (!it->second->casState(Chunk::TO_LOAD_MESH, Chunk::LOADING_MESH))
			return; // gone or already meshing
		chunk = it->second;
	}
	chunk->loadMesh(neighbors); // builds from the snapshot; no map access
	chunk->setState(Chunk::TO_RENDER);
	// Publishing to the main thread, which uploads the VAOs in update().
	std::lock_guard<std::mutex> lock(m_chunksMutex);
	m_chunksToUploadMesh.push(pos);
	m_chunksPendingUpload.insert(pos);
}

bool ChunkMap::isChunkInFrustum(Chunk* chunk, const Frustum& frustum) {
	ivec2 pos = chunk->getPosition();
	int height = chunk->getHeight() * Const::SECTION_HEIGHT;
	Box chunkBox = { { pos.x * Const::SECTION_SIDE, 0.f, pos.y * Const::SECTION_SIDE },
		{ Const::SECTION_SIDE, height, Const::SECTION_SIDE } };
	return !frustum.isBoxOutside(chunkBox);
}

bool ChunkMap::isChunkInFrustumApprox(ivec2 chunkPos, const Frustum& frustum) const {
	// Like isChunkInFrustum but with a fixed height for chunks that are not loaded yet.
	int height = Const::INIT_CHUNK_NB_SECTIONS * Const::SECTION_HEIGHT;
	Box chunkBox = { { chunkPos.x * Const::SECTION_SIDE, 0.f, chunkPos.y * Const::SECTION_SIDE },
		{ Const::SECTION_SIDE, height, Const::SECTION_SIDE } };
	return !frustum.isBoxOutside(chunkBox);
}
