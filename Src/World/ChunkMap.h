#pragma once

#include "World/Chunk.h"
#include "World/WorldConstants.h"
#include "View/Frustum.h"

#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>
#include <utility>
#include <mutex>
#include <atomic>

// Threading model
// ---------------
// Three threads touch a ChunkMap:
//   - the main (game-loop) thread renders and edits blocks,
//   - a dedicated loading thread streams chunks in and out,
//   - a short-lived updating thread rebuilds sections after bulk edits (explosions).
// m_chunksMutex serialises everything that touches the structure of m_chunks and
// the work queues. Only the loading thread ever inserts/erases chunks, which is why
// some reads on that thread can stay lock-free.
class ChunkMap {
public:
	static const int VIEW_DISTANCE, LOAD_DISTANCE, SIDE, CHUNKS_PER_LOAD;

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

	// ---- Loading thread ----
	void load(const Frustum& frustum);
	void unloadFarChunks();

	// ---- Updating thread (post-edit remesh) ----
	void reloadBlocksMeshes(const std::vector<ivec3>& blocks);
	bool canChunkBeEdited(ivec2 pos);

	// ---- Called from several threads ----
	// Locked. Reached from the main thread (block edits via Chunk::setBlock),
	// the loading thread (generation) and the updating thread (reloadBlocksMeshes).
	void reloadSectionMesh(ivec3 pos);
	// No internal locking. The const overloads are the loading thread's lock-free neighbour
	// reads (safe because only the loading thread changes the map structure); the non-const
	// overloads are used by the main thread while it already holds m_chunksMutex.
	Chunk& getChunk(ivec2 pos);
	const Chunk& getChunk(ivec2 pos) const;
	Section& getSection(ivec3 pos);
	const Section& getSection(ivec3 pos) const;
	// Called from whatever thread changes a Chunk's state; only updates the advisory counters.
	void onChangeChunkState(Chunk& chunk, Chunk::State nextState);

private:
	struct Comp_ivec2 {
		size_t operator()(const ivec2& vec) const {
			return std::hash<int>()(vec.x) ^ (std::hash<int>()(vec.y) << 1);
		}
		bool operator()(const ivec2& a, const ivec2& b) const {
			return a.x == b.x && a.y == b.y;
		}
	};

	// shared_ptr so the main thread can keep a chunk alive while editing it, even if the loading
	// thread erases it from the map in the meantime.
	std::unordered_map<ivec2, std::shared_ptr<Chunk>, Comp_ivec2, Comp_ivec2> m_chunks;
	std::queue<ivec2> m_chunksToUploadMesh;
	std::queue<ivec3> m_sectionsToLoadMesh;
	std::queue<ivec3> m_sectionsToReUploadMesh;
	ivec2 m_center;
	mutable std::mutex m_chunksMutex; // guards the structure of m_chunks and the two queues above
	bool m_mustStop = false;

	int m_renderedChunks = 0; // written and read only on the main thread (render / getRenderedChunks)
	// Updated from several threads via onChangeChunkState(), some of those calls unlocked, so the
	// counters are atomic (no lost increments, well-defined reads). A multi-bucket sum is still not
	// a consistent snapshot - it can be off by the transitions in flight - which is fine for a debug overlay.
	std::array<std::atomic<int>, Chunk::STATE_SIZE> m_countChunks{};

	// Caller must already hold m_chunksMutex
	std::unordered_map<ivec2, std::shared_ptr<Chunk>, Comp_ivec2, Comp_ivec2>::const_iterator
		hasBlock(ivec3 globalPos, bool canSurpass = false) const;
	// Called in the loading thread
	bool isInLoadDistance(ivec2 pos) const;
	void loadBlocks(ivec2 pos);
	void loadChunkMesh(ivec2 pos);
	// No lock needed
	bool isChunkInFrustum(ivec2 chunkPos, const Frustum& frustum);
};
