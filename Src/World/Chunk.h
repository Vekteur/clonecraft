#pragma once

#include "Section.h"
#include "Generator/ChunkGenerationInfo.h"
#include "WorldConstants.h"

#include <vector>
#include <map>
#include <atomic>
#include <mutex>

class ChunkMap;

class Chunk {
public:
	enum State {
		TO_LOAD_BLOCKS,
		LOADING_BLOCKS,
		TO_LOAD_MESH,
		LOADING_MESH,
		TO_RENDER,
		TO_RELEASE_MESH,
		TO_REMOVE,
		STATE_SIZE
	};

	Chunk(ChunkMap* const chunkMap = nullptr, ivec2 position = ivec2{0, 0});
	~Chunk();

	void setBlock(ivec3 pos, Block block);
	Block getBlock(ivec3 pos) const;
	// Sets just the light byte at pos, but only if its section already exists; returns whether it did.
	// Used by the cross-chunk light spill, which must never create an empty air section just to hold
	// light.
	void loadBlocks();
	void loadMesh(const NeighborChunks& neighbors);
	void uploadMesh();
	void releaseMesh();
	State getState() const;
	void setState(State state);
	// Atomically moves from expected to desired and returns whether it won. Loading workers use
	// it to claim a chunk so two of them never load the same one.
	bool casState(State expected, State desired);
	ivec2 getPosition() const;
	void render(const DefaultRenderer& defaultRenderer) const;
	void render(const WaterRenderer& waterRenderer) const;
	ChunkGenerationInfo& chunkInfo();
	const ChunkGenerationInfo& chunkInfo() const;

	// Sections are sparse: only existing ones are stored, keyed by their vertical index (which may be
	// negative). A missing section reads as air. Every one of these locks m_sectionsMutex.
	bool hasSection(int sectionY) const;
	Section& getSection(int sectionY);             // section must exist
	const Section& getSection(int sectionY) const; // section must exist
	std::map<int, Section>& getSections();  // returns the map of sections, for iteration
	Section* tryGetSection(int sectionY); // nullptr if it does not exist
	const Section* tryGetSection(int sectionY) const; // nullptr if it does not exist
	Section& getOrCreateSection(int sectionY);
	int minSectionY() const; // 0 if the chunk has no sections
	int maxSectionY() const; // 0 if the chunk has no sections

private:
	ChunkMap* const p_chunkMap{ nullptr };
	const vec2 m_position;
	ChunkGenerationInfo m_chunkGenerationInfo;

	std::atomic<State> m_state{ STATE_SIZE };

	// std::map is node-based, so a Section keeps a fixed address once inserted: pointers the loading
	// thread holds while meshing stay valid when another thread adds sections. m_sectionsMutex guards
	// every access to the map (find/insert/iterate); the keys may be sparse and negative. Sections
	// are never removed, so once hasSection() is true it stays true.
	std::map<int, Section> m_sections;
	mutable std::mutex m_sectionsMutex;
};