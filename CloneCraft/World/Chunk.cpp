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
	glDeleteBuffers(1, &EBO);
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
				if (m_blocks.at(ivec3{ i, j, k }) == 0)
					continue;

				vec3 globalPos{ i + m_position.x * CHUNK_SIDE, j, k + m_position.y * CHUNK_SIDE };

				if (!isInChunk(ivec3{ i + 1, j, k }) || m_blocks.at(ivec3{ i + 1, j, k }) == 0)
					for (GLfloat c : getFace(globalPos, faceX1))
						m_faces.push_back(c);

				if (!isInChunk(ivec3{ i - 1, j, k }) || m_blocks.at(ivec3{ i - 1, j, k }) == 0)
					for (GLfloat c : getFace(globalPos, faceX0))
						m_faces.push_back(c);

				if (!isInChunk(ivec3{ i, j + 1, k }) || m_blocks.at(ivec3{ i, j + 1, k }) == 0)
					for (GLfloat c : getFace(globalPos, faceY1))
						m_faces.push_back(c);

				if (!isInChunk(ivec3{ i, j - 1, k }) || m_blocks.at(ivec3{ i, j - 1, k }) == 0)
					for (GLfloat c : getFace(globalPos, faceY0))
						m_faces.push_back(c);

				if (!isInChunk(ivec3{ i, j, k + 1 }) || m_blocks.at(ivec3{ i, j, k + 1 }) == 0)
					for (GLfloat c : getFace(globalPos, faceZ1))
						m_faces.push_back(c);

				if (!isInChunk(ivec3{ i, j, k - 1 }) || m_blocks.at(ivec3{ i, j, k - 1 }) == 0)
					for (GLfloat c : getFace(globalPos, faceZ0))
						m_faces.push_back(c);
			}

	for (int i = 0; i < m_faces.size() / 20; ++i) // Each 4 segments
		for (GLuint index : indices) // Add the indices with an offset of 4 * i 
			m_indices.push_back(index + 4 * i);

	// Create and bind the VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// The VBO stores the vertices
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_faces.size(), &m_faces[0], GL_STREAM_DRAW);

	// The EBO stores the indices
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * m_indices.size(), &m_indices[0], GL_STREAM_DRAW);

	// Attributes of the VAO
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	/*for (int i = 0; i < m_faces.size() / 5; ++i)
	{
		for (int j = 0; j < 5; j++)
		{
			std::cout << m_faces[i * 5 + j] << ' ';
		}
		std::cout << '\n';
	}*/
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
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	assert(glGetError() == GL_NO_ERROR);
}

bool Chunk::isInChunk(ivec3 block)
{
	return 0 <= block.x && block.x < CHUNK_SIDE && 0 <= block.y && block.y < CHUNK_HEIGHT && 0 <= block.z && block.z < CHUNK_SIDE;
}

std::array<GLfloat, 20> Chunk::getFace(glm::ivec3 pos, const std::array<GLfloat, 12>& face)
{
	std::array<GLfloat, 20> finalFace;
	// Each vertex
	for (int i = 0; i < 4; ++i)
	{
		// Compute coordinates of the vertex
		for (int j = 0; j < 3; ++j)
			finalFace[5 * i + j] = face[3 * i + j] + pos[j];
		// Compute texture coordinates of the vertex
		for (int j = 0; j < 2; ++j)
			finalFace[5 * i + (j + 3)] = textureCoords[2 * i + j];
	}
	return finalFace;
}

const std::array<GLfloat, 6> Chunk::indices{
	0, 1, 2, 
	2, 3, 0
};

const std::array<GLfloat, 8> Chunk::textureCoords{
	0.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f
};

const std::array<GLfloat, 12> Chunk::faceX0{
	0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f
};

const std::array<GLfloat, 12> Chunk::faceX1{
	1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 0.0f
};

const std::array<GLfloat, 12> Chunk::faceY0{
	0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f
};

const std::array<GLfloat, 12> Chunk::faceY1{
	1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 0.0f
};

const std::array<GLfloat, 12> Chunk::faceZ0{
	1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f
};

const std::array<GLfloat, 12> Chunk::faceZ1{
	0.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 0.0f, 1.0f
};