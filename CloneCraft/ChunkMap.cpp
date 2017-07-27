#include "ChunkMap.h"
#include "ResManager.h"

ChunkMap::ChunkMap()
{
}


ChunkMap::~ChunkMap()
{
}

void ChunkMap::load(ivec2 position)
{
	loadBlocks(ivec2{ position.x, position.y }); // Base case : load the blocks of the center chunk
	for (int d = 1; d <= RENDER_DISTANCE + 1; ++d)
	{
		// Load the blocks in the chunks at distance d
		for (int x = position.x - d; x < position.x + d; ++x) // Top left to top right
			loadBlocks(ivec2{ x, position.y + d });
		for (int y = position.y + d; y > position.y - d; --y) // Top right to bottom right
			loadBlocks(ivec2{ position.x + d, y });
		for (int x = position.x + d; x > position.x - d; --x) // Bottom right to bottom left
			loadBlocks(ivec2{ x, position.y - d });
		for (int y = position.y - d; y < position.y + d; ++y) // Bottom left to top left
			loadBlocks(ivec2{ position.x - d, y });

		// load the faces in the chunks at distance d - 1
		int c = d - 1;
		for (int x = position.x - c; x < position.x + c; ++x) // Top left to top right
			loadFaces(ivec2{ x, position.y + c });
		for (int y = position.y + c; y > position.y - c; --y) // Top right to bottom right
			loadFaces(ivec2{ position.x + c, y });
		for (int x = position.x + c; x > position.x - c; --x) // Bottom right to bottom left
			loadFaces(ivec2{ x, position.y - c });
		for (int y = position.y - c; y < position.y + c; ++y) // Bottom left to top left
			loadFaces(ivec2{ position.x - c, y });
	}
	if(RENDER_DISTANCE >= 1)
		loadFaces(ivec2{ position.x, position.y }); // Load the faces in the center chunk
}

void ChunkMap::loadBlocks(ivec2 pos)
{
	if (m_chunks.find(pos) == m_chunks.end()) // Chunk not in the ChunkMap
		m_chunks.emplace(pos, Chunk{ this, pos });
	if(!m_chunks[pos].hasLoadedBlocks())
		m_chunks[pos].loadBlocks(); // Load the blocks
}

void ChunkMap::loadFaces(ivec2 pos)
{
	if (m_chunks.find(pos) == m_chunks.end()) // Chunk not in the ChunkMap
		m_chunks.emplace(pos, Chunk{ this, pos });
	if (!m_chunks[pos].hasLoadedFaces())
		m_chunks[pos].loadFaces(); // Load the faces
}

void ChunkMap::render()
{
	for (auto& c : m_chunks)
		if(c.second.hasLoadedFaces())
			c.second.render(ResManager::getShader("cube"), ResManager::getTexture("stone"));
}

Chunk& ChunkMap::getChunk(ivec2 pos)
{
	return m_chunks[pos];
}

Section& ChunkMap::getSection(ivec3 pos)
{
	ivec2 chunkPos{ pos.x, pos.z };
	return getChunk(chunkPos).getSection(pos.y);
}
