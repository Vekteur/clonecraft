#pragma once

#include "Chunk.h"
#include "WorldConstants.h"

#include <unordered_map>
#include <memory>
#include <utility>
#include <mutex>

class ChunkMap
{
public:
	static const int VIEW_DISTANCE{ 16 }, LOAD_DISTANCE{ VIEW_DISTANCE + 1 }, SIDE{ (2 * VIEW_DISTANCE + 1) * Const::CHUNK_SIDE };

	struct Comp_ivec2
	{
		size_t operator()(const ivec2& vec) const
		{
			return std::hash<int>()(vec.x) ^ (std::hash<int>()(vec.y) << 1);
		}

		bool operator()(const ivec2& a, const ivec2& b) const
		{
			return a.x == b.x && a.y == b.y;
		}
	};

	ChunkMap(ivec2 center = ivec2{ 0, 0 });
	~ChunkMap();

	void load();
	void update();
	void render();
	void setCenter(ivec2 center);
	ivec2 getCenter();

	Chunk& getChunk(ivec2 pos);
	Section& getSection(ivec3 pos);
	void unloadFarChunks();
	int getSize();

private:
	std::unordered_map<ivec2, std::unique_ptr<Chunk>, Comp_ivec2, Comp_ivec2> m_chunks;
	ivec2 m_center;
	ivec2 m_newCenter;
	std::mutex m_deleteChunksMutex;

	void loadBlocks(ivec2 pos);
	void loadFaces(ivec2 pos);
	bool isInChunkMap(ivec2 pos);
};

