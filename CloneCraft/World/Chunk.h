#pragma once

#include "Section.h"
#include "ChunkGenerator.h"
#include "WorldConstants.h"

#include <vector>

class Chunk {
public:
	enum State {
		TO_LOAD_BLOCKS,
		TO_LOAD_FACES,
		TO_RENDER,
		TO_UNLOAD_VAOS,
		TO_REMOVE,
		STATE_SIZE
	};

	Chunk(ChunkMap* const chunkMap = nullptr, ivec2 position = ivec2{0, 0});
	~Chunk();

	void setBlock(ivec3 pos, Block block);
	Block getBlock(ivec3 pos) const;
	void loadBlocks();
	void loadFaces();
	void loadVAOs();
	void unloadVAOs();
	State getState() const;
	void setState(State state);
	ivec2 getPosition() const;
	int getHeight() const;
	void render(const DefaultRenderer& defaultRenderer) const;
	void render(const WaterRenderer& waterRenderer) const;
	ChunkGenerator& getChunkGenerator();
	const ChunkGenerator& getChunkGenerator() const;
	std::vector<Section>& getSections();

	bool isInChunk(ivec3 globalPos) const;
	Section& getSection(int height);
	const Section& getSection(int height) const;

private:
	ChunkMap* const p_chunkMap{ nullptr };
	const vec2 m_position;
	ChunkGenerator m_chunkGenerator;
	State m_state{ STATE_SIZE };

	std::vector<Section> m_sections;
};