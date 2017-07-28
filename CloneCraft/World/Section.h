#pragma once

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <GlmCommon.h>

#include <array>
#include <vector>

#include "Array3D.h"
#include "Shader.h"
#include "Texture.h"

class ChunkMap;

class Section
{
public:
	Section(ChunkMap* const chunkMap, ivec3 position = ivec3{ 0, 0, 0 });
	~Section();

	void loadBlocks();
	void loadFaces();
	void render(Shader &shader, Texture2D &texture) const;

	int getBlock(ivec3 pos) const;

	static const int SIDE{ 16 }, HEIGHT{ 16 };

private:
	ChunkMap* const p_chunkMap{ nullptr };
	const ivec3 m_position;

	Array3D<GLuint> m_blocks;
	GLuint VAO, VBO, EBO;
	int indicesNb{ 0 };

	bool isInSection(ivec3 block);
	bool isAir(ivec3 block);
	static std::array<GLfloat, 20> getFace(glm::ivec3 pos, const std::array<GLfloat, 12>& face);

	static const std::array<GLuint, 6> rectIndices;
	static const std::array<GLfloat, 8> textureCoords;

	static const std::array<GLfloat, 12> faceX0;
	static const std::array<GLfloat, 12> faceX1;
	static const std::array<GLfloat, 12> faceY0;
	static const std::array<GLfloat, 12> faceY1;
	static const std::array<GLfloat, 12> faceZ0;
	static const std::array<GLfloat, 12> faceZ1;
};

