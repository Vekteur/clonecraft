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

class ChunkMap {
public:
	struct Comp_ivec2 {
		size_t operator()(const ivec2& vec) const {
			return std::hash<int>()(vec.x) ^ (std::hash<int>()(vec.y) << 1);
		}
		bool operator()(const ivec2& a, const ivec2& b) const {
			return a.x == b.x && a.y == b.y;
		}
	};

	static const int VIEW_DISTANCE, LOAD_DISTANCE, SIDE, CHUNKS_PER_LOAD;

	ChunkMap(ivec2 center = ivec2{ 0, 0 });
	~ChunkMap();

	void load(const Frustum& frustum);
	void update();
	void render(const Frustum& frustum, const DefaultRenderer* defaultRenderer = nullptr,
		const WaterRenderer* waterRenderer = nullptr);
	void setCenter(ivec2 center);
	ivec2 getCenter();
	void reloadSection(ivec3 pos);
	void reloadBlocks(const std::vector<ivec3>& blocks);

	void setBlock(ivec3 globalPos, Block block);
	Block getBlock(ivec3 globalPos) const;
	Chunk& getChunk(ivec2 pos);
	const Chunk& getChunk(ivec2 pos) const;
	Section& getSection(ivec3 pos);
	const Section& getSection(ivec3 pos) const;
	void unloadFarChunks();
	int size();
	void stop();

	void onChangeChunkState(Chunk& chunk, Chunk::State nextState);
	int chunksAtLeastInState(Chunk::State state);
	int chunksInState(Chunk::State state);
	int getRenderedChunks();

private:
	std::unordered_map<ivec2, std::unique_ptr<Chunk>, Comp_ivec2, Comp_ivec2> m_chunks;
	std::queue<ivec2> toLoadChunks;
	std::queue<ivec3> toReloadSections;
	ivec2 m_center;
	mutable std::mutex m_deleteChunksMutex;
	bool mustStop = false;

	int renderedChunks = 0;
	std::array<int, Chunk::STATE_SIZE> countChunks;

	std::unordered_map<ivec2, std::unique_ptr<Chunk>, Comp_ivec2, Comp_ivec2>::const_iterator
		hasBlock(ivec3 globalPos, bool canSurpass = false) const;
	bool isInLoadDistance(ivec2 pos) const;
	void loadBlocks(ivec2 pos);
	void loadFaces(ivec2 pos);
	bool isChunkInFrustum(ivec2 chunkPos, const Frustum& frustum);
};

