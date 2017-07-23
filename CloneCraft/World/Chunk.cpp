#include "Chunk.h"

Chunk::Chunk(ivec2 position)
	: m_position{ position }
{
	for (int i = 0; i < SECTION_HEIGHT; ++i)
		sections[i] = std::move(std::make_unique<Section>(ivec3{ m_position.x, i, m_position.y }));
}

Chunk::~Chunk()
{
}

void Chunk::load()
{
	for (int i = 0; i < SECTION_HEIGHT; ++i)
		sections[i]->load();
}

void Chunk::render(Shader & shader, Texture2D & texture)
{
	for (int i = 0; i < SECTION_HEIGHT; ++i)
		sections[i]->render(shader, texture);
}
