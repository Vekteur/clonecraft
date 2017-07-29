#include "ChunkMap.h"
#include "ResManager.h"
#include "Debug.h"

#include <GLFW\glfw3.h>

ChunkMap::ChunkMap(ivec2 center) : m_center{ center }
{
}


ChunkMap::~ChunkMap()
{
}

void ChunkMap::load(GLFWwindow* window)
{
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}

	std::cout << "Begin loading" << '\n';
	//lock.lock();

	loadBlocks(ivec2{ m_center.x, m_center.y }); // Base case : load the blocks of the center chunk
	for (int d = 1; d <= RENDER_DISTANCE + 1; ++d)
	{
		// Load the blocks in the chunks at distance d
		for (int x = m_center.x - d; x < m_center.x + d; ++x) // Top left to top right
			loadBlocks(ivec2{ x, m_center.y + d });
		for (int y = m_center.y + d; y > m_center.y - d; --y) // Top right to bottom right
			loadBlocks(ivec2{ m_center.x + d, y });
		for (int x = m_center.x + d; x > m_center.x - d; --x) // Bottom right to bottom left
			loadBlocks(ivec2{ x, m_center.y - d });
		for (int y = m_center.y - d; y < m_center.y + d; ++y) // Bottom left to top left
			loadBlocks(ivec2{ m_center.x - d, y });

		// load the faces in the chunks at distance d - 1
		int c = d - 1;
		for (int x = m_center.x - c; x < m_center.x + c; ++x) // Top left to top right
			loadFaces(ivec2{ x, m_center.y + c });
		for (int y = m_center.y + c; y > m_center.y - c; --y) // Top right to bottom right
			loadFaces(ivec2{ m_center.x + c, y });
		for (int x = m_center.x + c; x > m_center.x - c; --x) // Bottom right to bottom left
			loadFaces(ivec2{ x, m_center.y - c });
		for (int y = m_center.y - c; y < m_center.y + c; ++y) // Bottom left to top left
			loadFaces(ivec2{ m_center.x - c, y });
		std::cout << "Distance " << d << " loaded" << '\n';
	}
	if (RENDER_DISTANCE >= 1)
		loadFaces(ivec2{ m_center.x, m_center.y }); // Load the faces in the center chunk

	//lock.unlock();
	glFlush();
	std::cout << "End loading" << '\n';
}

void ChunkMap::loadVAOs()
{
	for (int x = m_center.x - RENDER_DISTANCE; x <= m_center.x + RENDER_DISTANCE; ++x)
		for (int y = m_center.y - RENDER_DISTANCE; y <= m_center.y + RENDER_DISTANCE; ++y)
			m_chunks[ivec2{ x, y }]->loadVAOs();
}

void ChunkMap::loadBlocks(ivec2 pos)
{
	if (m_chunks.find(pos) == m_chunks.end()) // Chunk not in the ChunkMap
		m_chunks.emplace(pos, std::make_unique<Chunk>(this, pos));
	if(!m_chunks[pos]->hasLoadedBlocks())
		m_chunks[pos]->loadBlocks(); // Load the blocks
}

void ChunkMap::loadFaces(ivec2 pos)
{
	if (m_chunks.find(pos) == m_chunks.end()) // Chunk not in the ChunkMap
		m_chunks.emplace(pos, std::make_unique<Chunk>(this, pos));
	if (!m_chunks[pos]->hasLoadedFaces())
		m_chunks[pos]->loadFaces(); // Load the faces
}

void ChunkMap::render()
{
	for (auto& c : m_chunks)
		if(c.second->hasLoadedFaces())
			c.second->render(ResManager::getShader("cube"), ResManager::getTexture("stone"));
}

void ChunkMap::setCenter(ivec2 center)
{
	m_center = center;
}

ivec2 ChunkMap::getCenter()
{
	return m_center;
}

Chunk& ChunkMap::getChunk(ivec2 pos)
{
	return *m_chunks[pos].get();
}

Section& ChunkMap::getSection(ivec3 pos)
{
	ivec2 chunkPos{ pos.x, pos.z };
	return getChunk(chunkPos).getSection(pos.y);
}

int ChunkMap::getSize()
{
	return m_chunks.size();
}
