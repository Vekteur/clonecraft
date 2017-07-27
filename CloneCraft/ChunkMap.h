#pragma once

#include "Chunk.h"

#include <unordered_map>
#include <memory>
#include <utility>

class ChunkMap
{
public:
	static const int RENDER_DISTANCE{ 4 };

	struct KeyComp
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

	ChunkMap();
	~ChunkMap();

	void load(ivec2 position);
	void render();

	Chunk& getChunk(ivec2 pos);
	Section& getSection(ivec3 pos);

private:
	std::unordered_map<ivec2, Chunk, KeyComp, KeyComp> m_chunks;

	void loadBlocks(ivec2 pos);
	void loadFaces(ivec2 pos);
};

