#pragma once

#include <glad\glad.h>
#include <GlmCommon.h>

#include <array>
#include <vector>

#include "Array3D.h"
#include "Shader.h"
#include "Texture.h"
#include "Dir3D.h"

class ChunkMap;
class Chunk;

class Section
{
public:
	Section(ChunkMap* const chunkMap, Chunk* const chunk, ivec3 position = ivec3{ 0, 0, 0 });
	~Section();

	void loadBlocks();
	void loadFaces();
	void loadVAOs();
	void unloadVAOs();
	void render(Shader &shader, Texture2D &texture) const;

	GLuint getBlock(ivec3 pos) const;

private:
	ChunkMap* const p_chunkMap{ nullptr };
	Chunk* const p_chunk{ nullptr };
	const ivec3 m_position;

	Array3D<GLuint> m_blocks;
	GLuint VAO, VBO, EBO;
	GLuint indicesNb{ 0 };

	bool isInSection(ivec3 block);
	GLuint getNearBlock(ivec3 block);
	static std::array<GLfloat, 20> getFace(glm::ivec3 pos, const std::array<GLfloat, 12>& face);

	static const std::array<GLuint, 6> rectIndices;
	static const std::array<GLfloat, 8> textureCoords;
	static const std::array<std::array<GLfloat, 12>, Dir3D::SIZE> dirToFace;
};