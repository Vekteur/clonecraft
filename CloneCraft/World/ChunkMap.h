#pragma once

#include "Chunk.h"

#include <unordered_map>
#include <memory>
#include <utility>
#include <mutex>

struct GLFWwindow;

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

	ChunkMap(ivec2 center = ivec2{ 0, 0 });
	~ChunkMap();

	void load(GLFWwindow* window);
	void render();
	void setCenter(ivec2 center);
	ivec2 getCenter();

	Chunk& getChunk(ivec2 pos);
	Section& getSection(ivec3 pos);

private:
	std::unordered_map<ivec2, std::unique_ptr<Chunk>, KeyComp, KeyComp> m_chunks;
	ivec2 m_center;
	//std::mutex lock; // Be sure that the load function is executed by one thread

	void loadBlocks(ivec2 pos);
	void loadFaces(ivec2 pos);
};

