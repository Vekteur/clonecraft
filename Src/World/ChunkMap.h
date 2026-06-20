#pragma once

#include "World/Chunk.h"
#include "World/WorldConstants.h"
#include "View/Frustum.h"

#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <utility>
#include <mutex>
#include <atomic>

// Threading model
// ---------------
//   - the main (game-loop) thread renders and edits blocks,
//   - one orchestrator thread keeps the sorted view list (m_toSelect) fresh and unloads far chunks,
//   - a pool of worker threads pull from the work pools and generate/mesh chunks,
//   - a short-lived updating thread rebuilds sections after bulk edits.
// m_chunksMutex guards m_chunks and every work pool/queue. The mesh build reads neighbors from a
// snapshot taken under the lock, so no thread ever reads the map structure lock-free; a chunk is
// claimed for loading with an atomic state CAS so two workers never load the same one.
class ChunkMap {
public:
	static const int VIEW_DISTANCE, LOAD_DISTANCE, SIDE, LOADING_WORKERS_COUNT;

	ChunkMap(ivec2 center = ivec2{ 0, 0 });
	~ChunkMap();

	// ---- Main (game-loop) thread ----
	void update();
	void render(const Frustum& frustum, const DefaultRenderer* defaultRenderer = nullptr,
		const WaterRenderer* waterRenderer = nullptr);
	void setBlock(ivec3 globalPos, Block block);
	Block getBlock(ivec3 globalPos) const;
	void setCenter(ivec2 center);
	ivec2 getCenter();
	void stop();
	// Debug-overlay accessors, read by WindowTextDrawer.
	int size();
	int chunksAtLeastInState(Chunk::State state);
	int chunksInState(Chunk::State state);
	int getRenderedChunks();

	// ---- Loading threads ----
	void refreshSelection(const Frustum& frustum); // orchestrator: rebuild the sorted view list
	void processNextTask();                         // worker: take and run one task
	void unloadFarChunks();                         // orchestrator

	// ---- Updating thread (post-edit remesh) ----
	void reloadBlocksMeshes(const std::vector<ivec3>& blocks);
	bool canChunkBeEdited(ivec2 pos);

	// ---- Called from several threads ----
	// Locked. Reached from the main thread (block edits via Chunk::setBlock),
	// the loading thread (generation) and the updating thread (reloadBlocksMeshes).
	void reloadSectionMesh(ivec3 pos);
	// No internal locking. The const overloads are the loading thread's lock-free neighbor
	// reads (safe because only the loading thread changes the map structure); the non-const
	// overloads are used by the main thread while it already holds m_chunksMutex.
	Chunk& getChunk(ivec2 pos);
	const Chunk& getChunk(ivec2 pos) const;
	Section& getSection(ivec3 pos);
	const Section& getSection(ivec3 pos) const;
	// Called from whatever thread changes a Chunk's state; only updates the advisory counters.
	void onChangeChunkState(Chunk::State previous, Chunk::State next);

private:
	struct Comp_ivec2 {
		size_t operator()(const ivec2& vec) const {
			return std::hash<int>()(vec.x) ^ (std::hash<int>()(vec.y) << 1);
		}
		bool operator()(const ivec2& a, const ivec2& b) const {
			return a.x == b.x && a.y == b.y;
		}
	};

	// shared_ptr so threads can keep a chunk alive while editing it, even if the orchestrator
	// thread erases it from the map in the meantime.
	std::unordered_map<ivec2, std::shared_ptr<Chunk>, Comp_ivec2, Comp_ivec2> m_chunks;
	std::queue<ivec2> m_chunksToUploadMesh;
	// Membership of m_chunksToUploadMesh: a chunk freshly meshed but not yet uploaded. A post-edit
	// section rebuild skips its own upload for these, since the pending full upload already carries it.
	std::unordered_set<ivec2, Comp_ivec2, Comp_ivec2> m_chunksPendingUpload;
	std::queue<ivec3> m_sectionsToReUploadMesh;

	// Work pools. A worker takes a task from the first non-empty pool in this order: load blocks,
	// then load mesh, then expand a selected chunk. The m_in* sets keep the queues duplicate-free.
	// m_toSelect is the orchestrator's sorted view list, consumed through m_selectCursor.
	std::queue<ivec2> m_toLoadBlocks;
	std::unordered_set<ivec2, Comp_ivec2, Comp_ivec2> m_inLoadBlocks;
	std::queue<ivec2> m_toLoadMeshes;
	std::unordered_set<ivec2, Comp_ivec2, Comp_ivec2> m_inLoadMeshes;
	std::vector<ivec2> m_toSelect;
	size_t m_selectCursor = 0;

	ivec2 m_center;
	mutable std::mutex m_chunksMutex; // guards m_chunks, the work pools and the upload queues
	bool m_mustStop = false;

	int m_renderedChunks = 0; // written and read only on the main thread (render / getRenderedChunks)
	// Updated from several threads via onChangeChunkState(), some of those calls unlocked, so the
	// counters are atomic (no lost increments, well-defined reads).
	std::array<std::atomic<int>, Chunk::STATE_SIZE> m_countChunks{};

	// Caller must already hold m_chunksMutex
	std::unordered_map<ivec2, std::shared_ptr<Chunk>, Comp_ivec2, Comp_ivec2>::const_iterator
		hasBlock(ivec3 globalPos, bool canSurpass = false) const;
	bool isInLoadDistance(ivec2 pos) const;
	// Worker task handlers
	void doSelectChunk(ivec2 pos);
	void doLoadBlocks(ivec2 pos);
	void doLoadMesh(ivec2 pos);
	// Caller must already hold m_chunksMutex
	void enqueueBlocksIfNeeded(ivec2 pos);
	void enqueueMeshIfReady(ivec2 pos);
	// No lock needed
	bool isChunkInFrustum(Chunk* chunk, const Frustum& frustum);
	bool isChunkInFrustumApprox(ivec2 chunkPos, const Frustum& frustum) const;
};
