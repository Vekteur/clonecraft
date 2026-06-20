#pragma once

#include "Section.h"
#include "Generator/ChunkGenerationInfo.h"
#include "WorldConstants.h"

#include <vector>
#include <deque>
#include <atomic>
#include <mutex>

class Chunk {
public:
	enum State {
		TO_LOAD_BLOCKS,
		TO_LOAD_MESH,
		TO_RENDER,
		TO_RELEASE_MESH,
		TO_REMOVE,
		STATE_SIZE
	};

	Chunk(ChunkMap* const chunkMap = nullptr, ivec2 position = ivec2{0, 0});
	~Chunk();

	void setBlock(ivec3 pos, Block block);
	Block getBlock(ivec3 pos) const;
	void loadBlocks();
	void loadMesh();
	void uploadMesh();
	void releaseMesh();
	State getState() const;
	void setState(State state);
	ivec2 getPosition() const;
	int getHeight() const;
	void render(const DefaultRenderer& defaultRenderer) const;
	void render(const WaterRenderer& waterRenderer) const;
	ChunkGenerationInfo& chunkInfo();
	const ChunkGenerationInfo& chunkInfo() const;

	bool isInChunk(ivec3 globalPos) const;
	Section& getSection(int height);
	const Section& getSection(int height) const;

private:
	ChunkMap* const p_chunkMap{ nullptr };
	const vec2 m_position;
	ChunkGenerationInfo m_chunkGenerationInfo;

	State m_state{ STATE_SIZE };

	// A deque keeps sections at fixed addresses as it grows, so pointers the loading thread holds
	// while meshing stay valid when the main thread adds sections. m_sectionsMutex guards that
	// growth; m_height lets the count be read without the lock. Sections are never removed.
	std::deque<Section> m_sections;
	std::atomic<int> m_height{ 0 };
	mutable std::mutex m_sectionsMutex;
};