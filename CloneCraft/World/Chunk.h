#pragma once

#include "Section.h"
#include "ChunkGenerator.h"
#include "WorldConstants.h"
#include <memory>

class Chunk
{
public:
	Chunk(ChunkMap* const chunkMap = nullptr, ivec2 position = ivec2{0, 0});
	~Chunk();

	void loadBlocks();
	void loadFaces();
	void loadVAOs();
	bool hasLoadedBlocks() const;
	bool hasLoadedFaces() const;
	bool hasLoadedVAOs() const;
	ivec2 getPosition() const;
	void render(Shader &shader, Texture2D &texture) const;
	ChunkGenerator& getChunkGenerator();

	Section& getSection(int height);

private:
	ChunkMap* const p_chunkMap{ nullptr };
	const vec2 m_position;
	ChunkGenerator m_chunkGenerator{ m_position };

	bool loadedFaces{ false };
	bool loadedBlocks{ false };
	bool loadedVAOs{ false };

	//ChunkGenerator m_chunkGenerator;
	std::array<std::unique_ptr<Section>, Const::CHUNK_NB_SECTIONS> m_sections;
};