#include "Chunk.h"

#include <iostream>

Chunk::Chunk(ivec2 position)
	:m_position{ position }, m_blocks{ ivec3{ CHUNK_SIDE, CHUNK_HEIGHT, CHUNK_SIDE } }
{
}


Chunk::~Chunk()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void Chunk::loadBlocks()
{
	for (int i = 0; i < CHUNK_SIDE; i++)
		for (int j = 0; j < CHUNK_HEIGHT; j++)
			for (int k = 0; k < CHUNK_SIDE; k++)
			{
				vec3 globalPos{ i + m_position.x * CHUNK_SIDE, j, k + m_position.y * CHUNK_SIDE };
				/// TODO GENERATION
				if (j < 64 || j == 100 || (i == 5 && j == 80 && k == 5))
					m_blocks.at(vec3{ i, j, k }) = 1;
				else
					m_blocks.at(vec3{ i, j, k }) = 0;
			}
}

void Chunk::loadFaces()
{
	for (int i = 0; i < CHUNK_SIDE; i++)
		for (int j = 0; j < CHUNK_HEIGHT; j++)
			for (int k = 0; k < CHUNK_SIDE; k++)
			{
				vec3 globalPos{ i + m_position.x * CHUNK_SIDE, j, k + m_position.y * CHUNK_SIDE };
				if (m_blocks.at(ivec3{ i, j, k }) == 0)
					continue;

				if (!isInChunk(ivec3{ i + 1, j, k }) || m_blocks.at(ivec3{ i + 1, j, k }) == 0)
					for (GLfloat face : getFaceX(globalPos, 1))
						m_faces.push_back(face);

				if (!isInChunk(ivec3{ i - 1, j, k }) || m_blocks.at(ivec3{ i - 1, j, k }) == 0)
					for (GLfloat face : getFaceX(globalPos))
						m_faces.push_back(face);

				if (!isInChunk(ivec3{ i, j + 1, k }) || m_blocks.at(ivec3{ i, j + 1, k }) == 0)
					for (GLfloat face : getFaceY(globalPos, 1))
						m_faces.push_back(face);

				if (!isInChunk(ivec3{ i, j - 1, k }) || m_blocks.at(ivec3{ i, j - 1, k }) == 0)
					for (GLfloat face : getFaceY(globalPos))
						m_faces.push_back(face);

				if (!isInChunk(ivec3{ i, j, k + 1 }) || m_blocks.at(ivec3{ i, j, k + 1 }) == 0)
					for (GLfloat face : getFaceZ(globalPos, 1))
						m_faces.push_back(face);

				if (!isInChunk(ivec3{ i, j, k - 1 }) || m_blocks.at(ivec3{ i, j, k - 1 }) == 0)
					for (GLfloat face : getFaceZ(globalPos))
						m_faces.push_back(face);
			}

	// Create and bind the VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// the VBO stores the vertices
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_faces.size(), &m_faces[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Attributes of the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	/*
	for (int i = 0; i < m_faces.size() / 5; ++i)
	{
		for (int j = 0; j < 5; j++)
		{
			std::cout << m_faces[i * 5 + j] << ' ';
		}
		std::cout << '\n';
	}*/
}

bool Chunk::isInChunk(ivec3 block)
{
	return 0 <= block.x && block.x < CHUNK_SIDE && 0 <= block.y && block.y < CHUNK_HEIGHT && 0 <= block.z && block.z < CHUNK_SIDE;
}

void Chunk::load()
{
	loadBlocks();
	loadFaces();
}

void Chunk::render(Shader &shader, Texture2D &texture)
{
	shader.use();
	glActiveTexture(GL_TEXTURE0);
	texture.bind();

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, m_faces.size() / 5);
	glBindVertexArray(0);

	assert(glGetError() == GL_NO_ERROR);
}

std::array<GLfloat, 30> Chunk::getFaceX(ivec3 pos, int offset)
{
	pos.x += offset;
	return std::array<GLfloat, 30> {
		0.0f + pos.x, 1.0f + pos.y, 1.0f + pos.z, 1.0f, 0.0f,
		0.0f + pos.x, 1.0f + pos.y, 0.0f + pos.z, 1.0f, 1.0f,
		0.0f + pos.x, 0.0f + pos.y, 0.0f + pos.z, 0.0f, 1.0f,
		0.0f + pos.x, 0.0f + pos.y, 0.0f + pos.z, 0.0f, 1.0f,
		0.0f + pos.x, 0.0f + pos.y, 1.0f + pos.z, 0.0f, 0.0f,
		0.0f + pos.x, 1.0f + pos.y, 1.0f + pos.z, 1.0f, 0.0f
	};
}

std::array<GLfloat, 30> Chunk::getFaceY(ivec3 pos, int offset)
{
	pos.y += offset;
	return std::array<GLfloat, 30> {
		0.0f + pos.x, 0.0f + pos.y, 0.0f + pos.z, 0.0f, 1.0f,
		1.0f + pos.x, 0.0f + pos.y, 0.0f + pos.z, 1.0f, 1.0f,
		1.0f + pos.x, 0.0f + pos.y, 1.0f + pos.z, 1.0f, 0.0f,
		1.0f + pos.x, 0.0f + pos.y, 1.0f + pos.z, 1.0f, 0.0f,
		0.0f + pos.x, 0.0f + pos.y, 1.0f + pos.z, 0.0f, 0.0f,
		0.0f + pos.x, 0.0f + pos.y, 0.0f + pos.z, 0.0f, 1.0f,
	};
}

std::array<GLfloat, 30> Chunk::getFaceZ(ivec3 pos, int offset)
{
	pos.z += offset;
	return std::array<GLfloat, 30> {
		0.0f + pos.x, 0.0f + pos.y, 0.0f + pos.z, 0.0f, 0.0f,
		1.0f + pos.x, 0.0f + pos.y, 0.0f + pos.z, 1.0f, 0.0f,
		1.0f + pos.x, 1.0f + pos.y, 0.0f + pos.z, 1.0f, 1.0f,
		1.0f + pos.x, 1.0f + pos.y, 0.0f + pos.z, 1.0f, 1.0f,
		0.0f + pos.x, 1.0f + pos.y, 0.0f + pos.z, 0.0f, 1.0f,
		0.0f + pos.x, 0.0f + pos.y, 0.0f + pos.z, 0.0f, 0.0f
	};
}
