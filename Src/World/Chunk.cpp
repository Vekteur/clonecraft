#include "Chunk.h"

#include "Maths/GlmCommon.h"
#include "ChunkMap.h"
#include "Generator/WorldGenerator.h"

Chunk::Chunk(ChunkMap* const chunkMap, ivec2 position)
	: p_chunkMap{ chunkMap }, m_position{ position } {
	setState(TO_LOAD_BLOCKS);
}

Chunk::~Chunk() {
	setState(STATE_SIZE);
}

void Chunk::setBlock(ivec3 pos, Block block) {
	int sectionY = pos.y / Const::SECTION_HEIGHT;
	if (sectionY >= getHeight()) {
		// Only adding sections needs the lock, to stay in sync with the loading thread's reads.
		// Publish m_height last so a reader seeing the new count also sees the finished section.
		int firstNewSection;
		{
			std::lock_guard<std::mutex> lock(m_sectionsMutex);
			firstNewSection = int(m_sections.size());
			for (int height = firstNewSection; height <= sectionY; ++height) {
				m_sections.emplace_back(p_chunkMap, this, ivec3{ m_position.x, height, m_position.y });
				m_height.store(int(m_sections.size()), std::memory_order_release);
			}
		}
		// reloadSectionMesh() takes other locks and re-enters our sections, so call it with
		// m_sectionsMutex released to avoid a deadlock.
		for (int height = firstNewSection; height <= sectionY; ++height)
			p_chunkMap->reloadSectionMesh({ m_position.x, height, m_position.y });
	}
	// Common case: the section already exists. Only this thread adds sections, so nothing can be
	// appending right now and indexing needs no lock.
	m_sections[sectionY].setBlock({ pos.x, pos.y % Const::SECTION_HEIGHT, pos.z }, block);
}

Block Chunk::getBlock(ivec3 pos) const {
	return getSection(pos.y / Const::SECTION_HEIGHT).getBlock({ pos.x, pos.y % Const::SECTION_HEIGHT, pos.z });
}

void Chunk::loadBlocks() {
	g_worldGenerator.loadChunk(*this);
	setState(TO_LOAD_MESH);
}

void Chunk::loadMesh() {
	// Grab the section pointers under the lock, then build the meshes without it (the slow part).
	// Sections added afterwards are meshed by the setBlock() call that created them.
	std::vector<Section*> sections;
	{
		std::lock_guard<std::mutex> lock(m_sectionsMutex);
		for (Section& section : m_sections)
			sections.push_back(&section);
	}
	for (Section* section : sections)
		section->loadMesh();
	setState(TO_RENDER);
}

void Chunk::uploadMesh() {
	for (auto& section : m_sections) {
		section.uploadMesh();
	}
}

void Chunk::releaseMesh() {
	for (auto& section : m_sections) {
		section.releaseMesh();
	}
	setState(TO_REMOVE);
}

Chunk::State Chunk::getState() const {
	return m_state;
}

void Chunk::setState(State state) {
	p_chunkMap->onChangeChunkState(*this, state);
	m_state = state;
}

ivec2 Chunk::getPosition() const {
	return m_position;
}

int Chunk::getHeight() const {
	return m_height.load(std::memory_order_acquire);
}

void Chunk::render(const DefaultRenderer & defaultRenderer) const {
	for (auto& section : m_sections) {
		section.render(defaultRenderer);
	}
}

void Chunk::render(const WaterRenderer& waterRenderer) const {
	for (auto& section : m_sections) {
		section.render(waterRenderer);
	}
}

ChunkGenerationInfo& Chunk::chunkInfo() {
	return m_chunkGenerationInfo;
}

const ChunkGenerationInfo& Chunk::chunkInfo() const {
	return m_chunkGenerationInfo;
}

bool Chunk::isInChunk(ivec3 globalPos) const {
	return 0 <= globalPos.x && globalPos.x < Const::SECTION_SIDE && 0 <= globalPos.y && 
		globalPos.y < getHeight() * Const::SECTION_HEIGHT &&
		0 <= globalPos.z && globalPos.z < Const::SECTION_SIDE;
}

Section& Chunk::getSection(int height) {
	// Lock so indexing can't race a section being added in setBlock(). The reference stays valid
	// after unlocking since the deque never moves or removes sections.
	std::lock_guard<std::mutex> lock(m_sectionsMutex);
	return m_sections[height];
}

const Section& Chunk::getSection(int height) const {
	std::lock_guard<std::mutex> lock(m_sectionsMutex);
	return m_sections[height];
}