#include "Chunk.h"

Chunk::Chunk(ChunkMap* const chunkMap, ivec2 position)
	: p_chunkMap{ chunkMap }, m_position{ position } {
	for (int i = 0; i < Const::CHUNK_NB_SECTIONS; ++i)
		m_sections[i] = std::make_unique<Section>(p_chunkMap, this, ivec3{ m_position.x, i, m_position.y });
}

Chunk::~Chunk() {
}

void Chunk::loadBlocks() {
	for (int i = 0; i < Const::CHUNK_NB_SECTIONS; ++i)
		m_sections[i]->loadBlocks();
	m_state = TO_LOAD_FACES;
}

void Chunk::loadFaces() {
	for (int i = 0; i < Const::CHUNK_NB_SECTIONS; ++i)
		m_sections[i]->loadFaces();
	m_state = TO_LOAD_VAOS;
}

void Chunk::loadVAOs() {
	for (int i = 0; i < Const::CHUNK_NB_SECTIONS; ++i)
		m_sections[i]->loadVAOs();
	m_state = TO_RENDER;
}

void Chunk::unloadVAOs() {
	for (int i = 0; i < Const::CHUNK_NB_SECTIONS; ++i)
		m_sections[i]->unloadVAOs();
	m_state = TO_REMOVE;
}

Chunk::State Chunk::getState() const {
	return m_state;
}

void Chunk::setState(State state) {
	m_state = state;
}

ivec2 Chunk::getPosition() const {
	return m_position;
}

void Chunk::render(Shader & shader, Texture2D & texture) const {
	for (int i = 0; i < Const::CHUNK_NB_SECTIONS; ++i)
		m_sections[i]->render(shader, texture);
}

ChunkGenerator& Chunk::getChunkGenerator() {
	return m_chunkGenerator;
}

Section& Chunk::getSection(int height) {
	return *m_sections[height].get();
}