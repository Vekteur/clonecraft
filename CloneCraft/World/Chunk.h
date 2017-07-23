#pragma once

#include "Section.h"
#include <memory>

class Chunk
{
public:
	Chunk(ivec2 position);
	~Chunk();

	void load();
	void render(Shader &shader, Texture2D &texture);

	static const int SECTION_HEIGHT{ 16 };
	static const int SIDE{ Section::SIDE }, HEIGHT{ Section::HEIGHT * Chunk::SECTION_HEIGHT };

private:
	vec2 m_position;

	std::array<std::unique_ptr<Section>, SECTION_HEIGHT> sections;
};