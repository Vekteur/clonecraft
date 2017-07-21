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
	std::vector<GLuint> m_indices;
	GLuint VAO, VBO, EBO;

	void loadBlocks();
	void loadFaces();
	static bool isInChunk(ivec3 block);

	static std::array<GLfloat, 20> getFace(glm::ivec3 pos, const std::array<GLfloat, 12>& face);

	static const std::array<GLfloat, 6> indices;
	static const std::array<GLfloat, 8> textureCoords;

	static const std::array<GLfloat, 12> faceX0;
	static const std::array<GLfloat, 12> faceX1;
	static const std::array<GLfloat, 12> faceY0;
	static const std::array<GLfloat, 12> faceY1;
	static const std::array<GLfloat, 12> faceZ0;
	static const std::array<GLfloat, 12> faceZ1;
};