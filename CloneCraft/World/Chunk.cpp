#include "Chunk.h"

#include "GlmCommon.h"
#include "ChunkMap.h"

Chunk::Chunk(ChunkMap* const chunkMap, ivec2 position)
	: p_chunkMap{ chunkMap }, m_position{ position }, m_chunkGenerator{ *this } {
	setState(TO_LOAD_BLOCKS);
	m_sections.reserve(Const::INIT_CHUNK_NB_SECTIONS);
	/*for (int i = 0; i < Const::CHUNK_NB_SECTIONS; ++i) {
		//m_sections[i] = std::make_unique<Section>(p_chunkMap, this, ivec3{ m_position.x, i, m_position.y });
		m_sections[i] = Section(p_chunkMap, this, ivec3{ m_position.x, i, m_position.y });
	}*/
}

Chunk::~Chunk() {
	setState(STATE_SIZE);
}

void Chunk::setBlock(ivec3 pos, Block block) {
	int sectionY = pos.y / Const::SECTION_HEIGHT;
	for (int height = int(m_sections.size()); height <= sectionY; ++height) {
		m_sections.emplace_back(p_chunkMap, this, ivec3{ m_position.x, height, m_position.y });
		p_chunkMap->reloadSection({ m_position.x, height, m_position.y });
	}
	getSection(sectionY).setBlock({ pos.x, pos.y % Const::SECTION_HEIGHT, pos.z }, block);
}

Block Chunk::getBlock(ivec3 pos) const {
	return getSection(pos.y / Const::SECTION_HEIGHT).getBlock({ pos.x, pos.y % Const::SECTION_HEIGHT, pos.z });
}

void Chunk::loadBlocks() {
	m_chunkGenerator.load();
	setState(TO_LOAD_FACES);
}

void Chunk::loadFaces() {
	for (auto& section : m_sections) {
		section.loadFaces();
	}
	setState(TO_RENDER);
}

void Chunk::loadVAOs() {
	for (auto& section : m_sections) {
		section.loadVAOs();
	}
}

void Chunk::unloadVAOs() {
	for (auto& section : m_sections) {
		section.unloadVAOs();
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
	return int(m_sections.size());
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

ChunkGenerator& Chunk::getChunkGenerator() {
	return m_chunkGenerator;
}

const ChunkGenerator& Chunk::getChunkGenerator() const {
	return m_chunkGenerator;
}

std::vector<Section>& Chunk::getSections() {
	return m_sections;
}

Section& Chunk::getSection(int height) {
	return m_sections[height];
}

const Section& Chunk::getSection(int height) const {
	return m_sections[height];
}