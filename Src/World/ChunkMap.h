#pragma once

#include "World/Chunk.h"
#include "World/WorldConstants.h"
#include "World/BlockEdit.h"
#include "View/Frustum.h"
#include "Maths/GlmCommon.h"

#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <utility>
#include <mutex>
#include <atomic>
#include <functional>

// Threading model
// ---------------
//   - the main (game-loop) thread renders and applies single-block edits (visible next frame),
//   - one orchestrator thread keeps the sorted view list (m_toSelect) fresh and unloads far chunks,
//   - a pool of worker threads pull from the work pools and generate/mesh chunks, and also run bulk
//     edits (compute + apply + remesh) the same way.
// m_chunksMutex guards m_chunks and every work pool/queue. The mesh build reads neighbors from a
// snapshot taken under the lock, so no thread ever reads the map structure lock-free; a chunk is
// claimed for loading with an atomic state CAS so two workers never load the same one. Touched
// chunks are kept alive with shared_ptr across the unlocked work so the orchestrator can erase them
// from the map without freeing them under a worker.
class ChunkMap {
public:
	static const int VIEW_DISTANCE, SIDE, LOADING_WORKERS_COUNT;

	// A bulk edit produces its block changes lazily on a worker thread (so the heavy computation never
	// stutters the frame). The function is the generator; it must not touch the ChunkMap.
	using EditGenerator = std::function<std::vector<BlockEdit>()>;

	ChunkMap(ivec2 center = ivec2{ 0, 0 });
	~ChunkMap();

	// ---- Main (game-loop) thread ----
	void update();
	void render(const Frustum& frustum, const DefaultRenderer* defaultRenderer = nullptr,
		const WaterRenderer* waterRenderer = nullptr);
	void setBlock(ivec3 globalPos, Block block);
	void applyEdits(const std::vector<BlockEdit>& edits);
	// Returns true if the edit was queued, false if it was dropped because one is already in flight.
	bool submitBulkEdit(EditGenerator generator);
	Block getBlock(ivec3 globalPos) const;
	void setCenter(ivec2 center);
	ivec2 getCenter();
	void stop();
	// Debug-overlay accessors, read by WindowTextDrawer.
	int size();
	int chunksAtLeastInState(Chunk::State state);
	int chunksInState(Chunk::State state);
	int getRenderedChunks();

	// ---- Worker threads ----
	void refreshSelection(const Frustum& frustum); // orchestrator: rebuild the sorted view list
	void processNextTask();                         // worker: take and run one task
	void unloadFarChunks();                         // orchestrator

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

	// Pending bulk-edit generators. Kept to at most one in flight: m_bulkEditRunning is set while a
	// worker owns one, so concurrent bulk edits can never remesh the same section at once.
	std::queue<EditGenerator> m_bulkEdits;
	bool m_bulkEditRunning = false;

	ivec2 m_center;
	mutable std::mutex m_chunksMutex; // guards m_chunks, the work pools and the upload queues
	// Guards the cross-chunk light spill (LightEngine::spillBorderLight) before each mesh, so two
	// workers can't write the same border light at once. Held only around the spill, not the mesh
	// build, and never alongside m_chunksMutex.
	std::mutex m_lightMutex;
	bool m_mustStop = false;

	int m_renderedChunks = 0; // written and read only on the main thread (render / getRenderedChunks)
	// Updated from several threads via onChangeChunkState(), some of those calls unlocked, so the
	// counters are atomic (no lost increments, well-defined reads).
	std::array<std::atomic<int>, Chunk::STATE_SIZE> m_countChunks{};

	// Iterator to the chunk owning globalPos if it has its blocks loaded, else end(). Caller must
	// already hold m_chunksMutex.
	std::unordered_map<ivec2, std::shared_ptr<Chunk>, Comp_ivec2, Comp_ivec2>::const_iterator
		findLoadedChunk(ivec3 globalPos) const;
	bool isInLoadDistance(ivec2 pos) const;
	// Worker task handlers
	void doSelectChunk(ivec2 pos);
	void doLoadBlocks(ivec2 pos);
	void doLoadMesh(ivec2 pos);
	void doBulkEdit(EditGenerator generator);
	void remeshSections(const std::vector<ivec3>& sections);
	bool canChunkBeEdited(ivec2 pos) const;
	// Caller must already hold m_chunksMutex
	void enqueueBlocksIfNeeded(ivec2 pos);
	void enqueueMeshIfReady(ivec2 pos);
	// No lock needed
	bool isChunkInFrustum(Chunk* chunk, const Frustum& frustum);
	bool isChunkInFrustumApprox(ivec2 chunkPos, const Frustum& frustum) const;
};
