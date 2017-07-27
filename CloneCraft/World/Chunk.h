#pragma once

#include "Section.h"
#include <memory>

class Chunk
{
public:
	Chunk(ChunkMap* const chunkMap = nullptr, ivec2 position = ivec2{0, 0});
	Chunk(Chunk&& c);
	~Chunk();

	void loadBlocks();
	void loadFaces();
	bool hasLoadedBlocks() const;
	bool hasLoadedFaces() const;
	void render(Shader &shader, Texture2D &texture) const;

	Section& getSection(int height);

	static const int SECTION_HEIGHT{ 16 };
	static const int SIDE{ Section::SIDE }, HEIGHT{ Section::HEIGHT * Chunk::SECTION_HEIGHT };

private:
	ChunkMap* const p_chunkMap{ nullptr };
	const vec2 m_position;

	bool loadedFaces{ false };
	bool loadedBlocks{ false };

	std::array<std::unique_ptr<Section>, SECTION_HEIGHT> m_sections;
};