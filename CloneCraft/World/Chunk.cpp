#include "Chunk.h"
#include "GlmCommon.h"
#include "ChunkMap.h"

Chunk::Chunk(ChunkMap* const chunkMap, ivec2 position)
	: p_chunkMap{ chunkMap }, m_position{ position } {
	setState(TO_LOAD_BLOCKS);
	for (int i = 0; i < Const::CHUNK_NB_SECTIONS; ++i) {
		m_sections[i] = std::make_unique<Section>(p_chunkMap, this, ivec3{ m_position.x, i, m_position.y });
	}
}

Chunk::~Chunk() {
	setState(STATE_SIZE);
}

void Chunk::loadBlocks() {
	for (auto& section : m_sections) {
		section->loadBlocks();
	}
	setState(TO_LOAD_FACES);
}

void Chunk::loadFaces() {
	for (auto& section : m_sections) {
		section->loadFaces();
	}
	setState(TO_LOAD_VAOS);
}

void Chunk::loadVAOs() {
	for (auto& section : m_sections) {
		section->loadVAOs();
	}
	setState(TO_RENDER);
}

void Chunk::unloadVAOs() {
	for (auto& section : m_sections) {
		section->unloadVAOs();
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

void Chunk::render(Shader & shader, Texture2D & texture) const {
	for (auto& section : m_sections) {
		section->render(shader, texture);
	}
}

ChunkGenerator& Chunk::getChunkGenerator() {
	return m_chunkGenerator;
}

Section& Chunk::getSection(int height) {
	return *m_sections[height].get();
}