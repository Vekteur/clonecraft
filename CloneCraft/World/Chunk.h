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
		EMPTY,
		LOADED_BLOCKS,
		LOADED_FACES,
		LOADED_VAOS,
		TO_UNLOAD_VAOS,
		TO_REMOVE
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
	void render(Shader &shader, Texture2D &texture) const;
	ChunkGenerator& getChunkGenerator();

	Section& getSection(int height);

private:
	ChunkMap* const p_chunkMap{ nullptr };
	const vec2 m_position;
	ChunkGenerator m_chunkGenerator{ m_position };
	State m_state{ EMPTY };

	std::array<std::unique_ptr<Section>, Const::CHUNK_NB_SECTIONS> m_sections;
};