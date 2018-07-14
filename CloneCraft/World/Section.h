#pragma once

#include <glad\glad.h>
#include <GlmCommon.h>

#include <array>
#include <vector>

#include "Array3D.h"
#include "Shader.h"
#include "Texture.h"
#include "Dir3D.h"
#include "Block.h"

class ChunkMap;
class Chunk;

struct Vertex {
	vec3 pos;
	GLuint texNorm;
	GLuint texID;
};

class Section
{
public:
	Section(ChunkMap* const chunkMap, Chunk* const chunk, ivec3 position = ivec3{ 0, 0, 0 });
	~Section();

	void loadBlocks();
	void loadFaces();
	void loadVAOs();
	void unloadVAOs();
	void render(Shader &shader) const;

	void setBlock(ivec3 pos, Block block);
	Block getBlock(ivec3 pos) const;

private:
	ChunkMap* const p_chunkMap{ nullptr };
	Chunk* const p_chunk{ nullptr };
	const ivec3 m_position;

	Array3D<Block> m_blocks;
	GLuint VAO, VBO, EBO;
	GLuint indicesNb{ 0 };
	bool empty = true;

	bool isInSection(ivec3 block);
	std::vector<Vertex> findFaces();

	static const std::array<GLuint, 6> rectIndices;
	static const std::array<ivec2, 4> textureCoords;
	static const std::array<std::array<vec3, 4>, Dir3D::SIZE> dirToFace;
};