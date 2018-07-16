#pragma once

#include "Section.h"
#include "ChunkGenerator.h"
#include "WorldConstants.h"

#include <memory>

class Chunk
{
public:

	enum State
	{
		TO_LOAD_BLOCKS,
		TO_LOAD_FACES,
		TO_LOAD_VAOS,
		TO_RENDER,
		TO_UNLOAD_VAOS,
		TO_REMOVE,
		STATE_SIZE
	};

	Chunk(ChunkMap* const chunkMap = nullptr, ivec2 position = ivec2{0, 0});
	~Chunk();

	void loadBlocks();
	void loadFaces();
	void loadVAOs();
	void unloadVAOs();
	State getState() const;
	void setState(State state);
	ivec2 getPosition() const;
	void render(DefaultRenderer& defaultRenderer, WaterRenderer& waterRenderer) const;
	ChunkGenerator& getChunkGenerator();

	Section& getSection(int height);

private:
	ChunkMap* const p_chunkMap{ nullptr };
	const vec2 m_position;
	ChunkGenerator m_chunkGenerator{ m_position };
	State m_state{ STATE_SIZE };

	std::array<std::unique_ptr<Section>, Const::CHUNK_NB_SECTIONS> m_sections;
};