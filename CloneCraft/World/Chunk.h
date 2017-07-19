#pragma once

#include <GL/glew.h>
#include <GLFW\glfw3.h>
#include <GlmCommon.h>

#include <array>
#include <vector>

#include "Array3D.h"
#include "Shader.h"
#include "Texture.h"

class Chunk
{
public:
	Chunk(ivec2 position);
	~Chunk();

	void load();
	void render(Shader &shader, Texture2D &texture);

private:

	static const int CHUNK_SIDE{ 16 }, CHUNK_HEIGHT{ CHUNK_SIDE * CHUNK_SIDE };

	vec2 m_position;
	Array3D<GLuint> m_blocks;
	std::vector<GLfloat> m_faces;
	GLuint VAO, VBO;

	void loadBlocks();
	void loadFaces();
	static bool isInChunk(ivec3 block);
	static std::array<GLfloat, 30> getFaceX(ivec3 pos, int offset = 0);
	static std::array<GLfloat, 30> getFaceY(ivec3 pos, int offset = 0);
	static std::array<GLfloat, 30> getFaceZ(ivec3 pos, int offset = 0);
};