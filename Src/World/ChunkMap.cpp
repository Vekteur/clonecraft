#include "ChunkMap.h"

#include "LightEngine.h"
#include "ResManager/ResManager.h"
#include "Util/DebugGL.h"
#include "Maths/Dir2D.h"
#include "Maths/Dir3D.h"
#include "Util/Array3D.h"
#include "Util/Logger.h"
#include "Maths/MiscMath.h"
#include "Maths/Converter.h"

#include <vector>
#include <array>
#include <algorithm>
#include <unordered_set>
#include <thread>
#include <chrono>

#ifndef CC_VIEW_DISTANCE
#define CC_VIEW_DISTANCE 32
#endif
const int ChunkMap::VIEW_DISTANCE{ CC_VIEW_DISTANCE };
const int ChunkMap::SIDE{ (2 * VIEW_DISTANCE + 1) * Const::SECTION_SIDE };
#ifndef CC_LOADING_WORKERS_COUNT
#define CC_LOADING_WORKERS_COUNT 4
#endif
const int ChunkMap::LOADING_WORKERS_COUNT{ CC_LOADING_WORKERS_COUNT };

namespace {
	constexpr std::array<ivec2, 9> NEIGHBORHOOD_3X3 = { {
		{ -1, -1 }, { 0, -1 }, { 1, -1 },
		{ -1,  0 }, { 0,  0 }, { 1,  0 },
		{ -1,  1 }, { 0,  1 }, { 1,  1 },
	} };
}

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
		ivec3 sectionPos = m_sectionsToReUploadMesh.front();
		m_sectionsToReUploadMesh.pop();
		// The chunk may have been unloaded between the remesh and now, so re-check before touching it.
		auto it = m_chunks.find(Converter::to2D(sectionPos));
		if (it != m_chunks.end() && it->second->getState() == Chunk::TO_RENDER &&
				it->second->hasSection(sectionPos.y)) {
			Section& section = it->second->getSection(sectionPos.y);
			// uploadMesh() frees the previous mesh before the upload.
			section.uploadMesh();
		}
	}
}

void ChunkMap::render(const Frustum& frustum, const std::vector<const Renderer*>& renderers) {
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

	for (const Renderer* renderer : renderers) {
		for (auto& chunk : chunks) {
			std::get<1>(chunk)->render(*renderer);
		}
	}
}

void ChunkMap::setBlock(ivec3 globalPos, Block block) {
	applyEdits({ { globalPos, block } });
}

bool allAirEdits(const std::vector<const BlockEdit*>& edits) {
	for (const BlockEdit* edit : edits)
		if (edit->block.id != +BlockID::AIR)
			return false;
	return true;
}

void ChunkMap::applyEdits(const std::vector<BlockEdit>& edits) {
	auto startTime = std::chrono::high_resolution_clock::now();

	if (edits.empty())
		return;

	auto timeSinceStart = [&]() {
		std::cout << "  - Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch()).count() - std::chrono::duration_cast<std::chrono::milliseconds>(startTime.time_since_epoch()).count() << " ms" << std::endl;
	};

	std::cout << "Grouping " << edits.size() << " by chunk" << std::endl;

	// Group the edits by chunk, then resolve those chunks once under the lock, keeping each alive
	// with a shared_ptr for the duration of the write (the orchestrator may unload it meanwhile).
	std::unordered_map<ivec2, std::vector<const BlockEdit*>, Comp_ivec2, Comp_ivec2> editsByChunk;
	for (const BlockEdit& edit : edits)
		editsByChunk[Converter::globalToChunk(edit.pos)].push_back(&edit);

	std::unordered_map<ivec2, std::shared_ptr<Chunk>, Comp_ivec2, Comp_ivec2> chunks;
	// Chunks the light update may touch: each edited chunk plus its eight surrounding neighbors.
	std::unordered_map<ivec2, std::shared_ptr<Chunk>, Comp_ivec2, Comp_ivec2> lightChunks;
	{
		std::lock_guard<std::mutex> lock(m_chunksMutex);
		auto pin = [&](ivec2 pos) {
			auto it = m_chunks.find(pos);
			if (it != m_chunks.end() && it->second->getState() >= Chunk::TO_LOAD_MESH)
				lightChunks.emplace(pos, it->second);
		};
		for (auto& [chunkPos, _] : editsByChunk) {
			auto it = m_chunks.find(chunkPos);
			if (it != m_chunks.end() && it->second->getState() >= Chunk::TO_LOAD_MESH)
				chunks.emplace(chunkPos, it->second);
			for (ivec2 off : NEIGHBORHOOD_3X3)
				pin(chunkPos + off);
		}
	}

	// TODO somewhere: all neighbors of a section that is updated must be created if they don't exist, 
	// even if it means creating empty sections. Otherwise sky light may not propagate from the sides.

	timeSinceStart();
	std::cout << "Applying " << edits.size() << " block edits" << std::endl;

	// Write the blocks without the global lock, grouped by section so each is looked up once.
	std::vector<LightEngine::LightEdit> lightEdits;
	std::vector<ivec3> editedSections;
	for (auto& [chunkPos, chunkEdits] : editsByChunk) {
		auto chunkIt = chunks.find(chunkPos);
		if (chunkIt == chunks.end())
			continue; // chunk not loaded (edge of the map, or unloaded), skip its edits
		Chunk& chunk = *chunkIt->second;
		std::unordered_map<int, std::vector<const BlockEdit*>> editsBySection;
		for (const BlockEdit* edit : chunkEdits)
			editsBySection[floorDiv(edit->pos.y, Const::SECTION_HEIGHT)].push_back(edit);
		for (auto& [sectionY, sectionEdits] : editsBySection) {
			Section* section = nullptr;
			if (allAirEdits(sectionEdits)) {
				section = chunk.tryGetSection(sectionY);
				if (section == nullptr)
					continue; // The section stays empty
			} else {
				section = &chunk.getOrCreateSection(sectionY);
			}
			editedSections.push_back({ chunkPos.x, sectionY, chunkPos.y });
			for (const BlockEdit* edit : sectionEdits) {
				ivec3 local = Converter::globalToInnerSection(edit->pos);
				// Keep the old block (its light feeds the relight) and write the new one with its light
				// cleared; the relighter fills it back in.
				Block oldBlock = section->getBlock(local);
				Block newBlock = edit->block;
				newBlock.light = { 0 };
				section->setBlock(local, newBlock);
				lightEdits.push_back({ edit->pos, oldBlock, newBlock });
			}
		}
	}

	timeSinceStart();
	std::cout << "Relighting " << lightEdits.size() << " edits" << std::endl;

	// Fix up the light around the edits and add the sections it changed to the remesh set below.
	// m_lightMutex keeps this from racing the other light writers.
	std::vector<ivec3> lightDirtySections;
	if (!lightEdits.empty()) {
		std::vector<std::pair<ivec2, Chunk*>> lightChunksRaw;
		lightChunksRaw.reserve(lightChunks.size());
		for (auto& [pos, chunk] : lightChunks)
			lightChunksRaw.emplace_back(pos, chunk.get());
		std::lock_guard<std::mutex> lightLock(m_lightMutex);
		LightEngine::updateEditLight(lightChunksRaw, lightEdits, lightDirtySections);
	}

	timeSinceStart();
	std::cout << "Determining the sections of " << edits.size() << " edits" << std::endl;

	// Remesh every section a change can affect: each edited section and its six face neighbors (a face
	// is hidden by the block on the other side). Expanding whole sections instead of each block may
	// rebuild a neighbor that didn't really change, but that's just a wasted rebuild, never a stale
	// mesh, and far cheaper than doing this per block.

	std::unordered_set<ivec3, Comp_ivec3, Comp_ivec3> sectionsToUpdate;
	for (const ivec3& section : editedSections) {
		sectionsToUpdate.insert(section);
		for (Dir3D::Dir dir : Dir3D::all())
			sectionsToUpdate.insert(section + Dir3D::to_ivec3(dir));
	}

	timeSinceStart();
	std::cout << "Remeshing " << sectionsToUpdate.size() << " sections" << std::endl;

	// Light can change well past the edited block (removing a torch darkens its whole radius), so add
	// every section the relight changed.
	for (const ivec3& section : lightDirtySections)
		sectionsToUpdate.insert(section);
	remeshSections({ sectionsToUpdate.begin(), sectionsToUpdate.end() });
	
	timeSinceStart();
}

bool ChunkMap::submitBulkEdit(EditGenerator generator) {
	std::lock_guard<std::mutex> lock(m_chunksMutex);
	// Keep at most one bulk edit pending or running, so two never remesh the same section at once.
	if (m_bulkEditRunning || !m_bulkEdits.empty())
		return false;
	m_bulkEdits.push(std::move(generator));
	return true;
}

Block ChunkMap::getBlock(ivec3 globalPos) const {
	std::lock_guard<std::mutex> lock(m_chunksMutex);
	auto chunkIt = findLoadedChunk(globalPos);
	if (chunkIt != m_chunks.end()) {
		ivec2 innerChunkPos = Converter::globalToInnerChunk2D(globalPos);
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
	enum { BULK, BLOCK, MESH, SELECT, NONE } type = NONE;
	ivec2 pos;
	EditGenerator bulkGenerator;
	{
		std::lock_guard<std::mutex> lock(m_chunksMutex);
		if (!m_bulkEdits.empty()) {
			// Bulk edits run first: they are user-initiated and the player wants to see the result. Max 1 worker.
			bulkGenerator = std::move(m_bulkEdits.front()); m_bulkEdits.pop();
			m_bulkEditRunning = true; type = BULK;
		} else if (!m_toLoadBlocks.empty()) {
			// Block loading runs next: these tasks have been added only when mesh tasks were not available,
			// so they should not starve them.
			pos = m_toLoadBlocks.front(); m_toLoadBlocks.pop(); m_inLoadBlocks.erase(pos); type = BLOCK;
		} else if (!m_toLoadMeshes.empty()) {
			pos = m_toLoadMeshes.front(); m_toLoadMeshes.pop(); m_inLoadMeshes.erase(pos); type = MESH;
		} else if (m_selectCursor < m_toSelect.size()) {
			pos = m_toSelect[m_selectCursor++]; type = SELECT;
		}
	}
	switch (type) {
		case BULK:   doBulkEdit(std::move(bulkGenerator)); break;
		case BLOCK:  doLoadBlocks(pos); break;
		case MESH:   doLoadMesh(pos);   break;
		case SELECT: doSelectChunk(pos); break;
		case NONE:   std::this_thread::sleep_for(std::chrono::milliseconds(1)); break;
	}
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

std::unordered_map<ivec2, std::shared_ptr<Chunk>, Comp_ivec2, Comp_ivec2>::const_iterator
ChunkMap::findLoadedChunk(ivec3 globalPos) const {
	auto chunkIt = m_chunks.find(Converter::globalToChunk(globalPos));
	if (chunkIt != m_chunks.end() && chunkIt->second->getState() >= Chunk::TO_LOAD_MESH)
		return chunkIt;
	else
		return m_chunks.end();
}

bool ChunkMap::isInLoadDistance(ivec2 pos) const {
	// Load a chunk iff some chunk in its 3x3 neighborhood is within view.
	ivec2 d = glm::max(ivec2{ 0, 0 }, glm::abs(pos - m_center) - ivec2{ 1, 1 });
	return d.x * d.x + d.y * d.y <= VIEW_DISTANCE * VIEW_DISTANCE;
}

void ChunkMap::doSelectChunk(ivec2 pos) {
	// A chunk's mesh needs every one of its eight neighbors' blocks (diagonals included, for corner
	// lighting), so queue blocks for the whole 3x3 around each selected chunk.
	std::lock_guard<std::mutex> lock(m_chunksMutex);
	for (ivec2 off : NEIGHBORHOOD_3X3)
		enqueueBlocksIfNeeded(pos + off);
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
	// This chunk's blocks just appeared, which may complete the mesh prerequisites of itself and of
	// every chunk it borders, diagonals included, so re-check the whole 3x3.
	std::lock_guard<std::mutex> lock(m_chunksMutex);
	for (ivec2 off : NEIGHBORHOOD_3X3)
		enqueueMeshIfReady(pos + off);
}

void ChunkMap::doLoadMesh(ivec2 pos) {
	std::shared_ptr<Chunk> chunk;
	std::array<std::shared_ptr<Chunk>, 9> keepAlive; // keep the 3x3 neighborhood alive during the build
	NeighborChunks neighbors{};
	{
		std::lock_guard<std::mutex> lock(m_chunksMutex);
		auto it = m_chunks.find(pos);
		if (it == m_chunks.end())
			return;
		// Snapshot the 3x3 chunk neighborhood (center included) so the build below never reads the map.
		// The diagonals feed corner lighting.
		for (ivec2 off : NEIGHBORHOOD_3X3) {
			auto neighborIt = m_chunks.find(pos + off);
			if (neighborIt != m_chunks.end()) {
				keepAlive[NeighborChunks::index(off.x, off.y)] = neighborIt->second;
				neighbors.at(off.x, off.y) = neighborIt->second.get();
			}
		}
		if (!it->second->casState(Chunk::TO_LOAD_MESH, Chunk::LOADING_MESH))
			return; // gone or already meshing
		chunk = it->second;
	}

	// Before meshing, settle light across this chunk's borders: push its light into the neighbors and
	// pull theirs in. The neighbors are at least TO_LOAD_MESH, so their light is ready to read. The
	// lock stops two spills racing on the same light bytes.
	std::array<Chunk*, Dir2D::SIZE> mutableNeighbors{};
	for (Dir2D::Dir dir : Dir2D::all()) {
		const ivec2 v = Dir2D::to_ivec2(dir);
		mutableNeighbors[dir] = keepAlive[NeighborChunks::index(v.x, v.y)].get();
	}
	std::vector<ivec3> lightDirtySections;
	{
		std::lock_guard<std::mutex> lightLock(m_lightMutex);
		LightEngine::spillBorderLight(*chunk, mutableNeighbors, lightDirtySections);
	}

	chunk->loadMesh(neighbors); // builds from the snapshot; no map access
	chunk->setState(Chunk::TO_RENDER);
	// Publishing to the main thread, which uploads the VAOs in update().
	{
		std::lock_guard<std::mutex> lock(m_chunksMutex);
		m_chunksToUploadMesh.push(pos);
		m_chunksPendingUpload.insert(pos);
	}
	// Rebuild the neighbor sections the spill brightened (this chunk's own are covered by the mesh we
	// just published). Runs after the publish so it can't race this chunk's upload. A neighbor not yet
	// meshed picks up the spilled light on its own when it meshes. The one gap: a neighbor meshing
	// right now may miss the spill and show a faint seam until its next rebuild. This is rare, and any
	// later edit fixes it.
	remeshSections(lightDirtySections);
}

void ChunkMap::doBulkEdit(EditGenerator generator) {
	// Compute the block changes off the lock, then apply + remesh through the same path a single edit uses.
	std::vector<BlockEdit> edits = generator();
	applyEdits(edits);
	std::lock_guard<std::mutex> lock(m_chunksMutex);
	m_bulkEditRunning = false;
}

void ChunkMap::remeshSections(const std::vector<ivec3>& sections) {
	// One section to rebuild, with everything it needs kept alive for the unlocked build below.
	struct MeshJob {
		ivec3 pos;
		std::shared_ptr<Chunk> chunk;
		std::array<std::shared_ptr<Chunk>, 9> keepAlive; // the 3x3 neighborhood, center included
		NeighborChunks neighbors{};
	};

	// Snapshot under the lock: which sections still exist on a rendered chunk, and the
	// neighbor chunks their border faces read from.
	std::vector<MeshJob> jobs;
	{
		std::lock_guard<std::mutex> lock(m_chunksMutex);
		for (ivec3 sectionPos : sections) {
			ivec2 pos2D = Converter::to2D(sectionPos);
			auto chunkIt = m_chunks.find(pos2D);
			if (chunkIt == m_chunks.end() || chunkIt->second->getState() != Chunk::TO_RENDER ||
					!chunkIt->second->hasSection(sectionPos.y))
				continue;
			if (!canChunkBeEdited(pos2D))
				continue;
			MeshJob job;
			job.pos = sectionPos;
			job.chunk = chunkIt->second;
			// Snapshot the 3x3 neighborhood (center included); the diagonals feed corner lighting.
			for (ivec2 off : NEIGHBORHOOD_3X3) {
				auto it = m_chunks.find(pos2D + off);
				if (it != m_chunks.end()) {
					job.keepAlive[NeighborChunks::index(off.x, off.y)] = it->second;
					job.neighbors.at(off.x, off.y) = it->second.get();
				}
			}
			jobs.push_back(std::move(job));
		}
	}

	// Build the meshes without the lock.
	for (MeshJob& job : jobs)
		job.chunk->getSection(job.pos.y).loadMesh(job.neighbors);

	// Publish the GPU uploads to the main thread under the lock.
	std::lock_guard<std::mutex> lock(m_chunksMutex);
	for (MeshJob& job : jobs) {
		// If the chunk still owes a full upload, that upload already carries this freshly rebuilt
		// section; a section upload would run after it and push an already-consumed empty mesh.
		if (m_chunksPendingUpload.count(Converter::to2D(job.pos)) == 0)
			m_sectionsToReUploadMesh.push(job.pos);
	}
}

bool ChunkMap::canChunkBeEdited(ivec2 pos) const {
	// A remesh reads the full 3x3 neighborhood (diagonals feed corner lighting), so all must exist.
	for (ivec2 off : NEIGHBORHOOD_3X3)
		if (m_chunks.find(pos + off) == m_chunks.end())
			return false;
	return true;
}

void ChunkMap::enqueueBlocksIfNeeded(ivec2 pos) {
	auto it = m_chunks.find(pos);
	bool needsBlocks = (it == m_chunks.end()) || (it->second->getState() == Chunk::TO_LOAD_BLOCKS);
	if (needsBlocks && m_inLoadBlocks.insert(pos).second)
		m_toLoadBlocks.push(pos);
}

void ChunkMap::enqueueMeshIfReady(ivec2 pos) {
	auto it = m_chunks.find(pos);
	if (it == m_chunks.end() || it->second->getState() != Chunk::TO_LOAD_MESH)
		return; // missing, still loading blocks, or already meshing/meshed
	// All eight neighbors' blocks must be ready: a face's corner lighting samples the diagonal chunks.
	for (ivec2 off : NEIGHBORHOOD_3X3) {
		auto neighborIt = m_chunks.find(pos + off);
		if (neighborIt == m_chunks.end() || neighborIt->second->getState() < Chunk::TO_LOAD_MESH)
			return; // a neighbor's blocks are not ready, so this mesh is not either
	}
	if (m_inLoadMeshes.insert(pos).second)
		m_toLoadMeshes.push(pos);
}

bool ChunkMap::isChunkInFrustum(Chunk* chunk, const Frustum& frustum) {
	ivec2 pos = chunk->getPosition();
	// Sparse sections span [minSectionY, maxSectionY]; the box covers that vertical range.
	int bottom = chunk->minSectionY() * Const::SECTION_HEIGHT;
	int height = (chunk->maxSectionY() + 1) * Const::SECTION_HEIGHT - bottom;
	Box chunkBox = { { pos.x * Const::SECTION_SIDE, float(bottom), pos.y * Const::SECTION_SIDE },
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
