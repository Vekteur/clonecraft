#include "Section.h"

#include "OctavePerlin.h"

#include <iostream>
#include <Windows.h>

Section::Section(ivec3 position)
	:m_position{ position }, m_blocks{ ivec3{ SIDE, HEIGHT, SIDE } }
{
}

Section::~Section()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void Section::loadBlocks()
{
	OctavePerlin noise(4, 0.5, 0.5);

	for (int x = 0; x < SIDE; x++)
		for (int z = 0; z < SIDE; z++)
		{
			ivec2 globalPos{ x + m_position.x * SIDE, z + m_position.z * SIDE };
			int height = 64 + floor(noise.getNoise(glm::vec2{ static_cast<float>(globalPos.x) / SIDE, static_cast<float>(globalPos.y) / SIDE }) * 4);
			for (int y = 0; y < HEIGHT; y++)
			{
				if (y + m_position.y * HEIGHT <= height)
					m_blocks.at(vec3{ x, y, z }) = 1;
				else
					m_blocks.at(vec3{ x, y, z }) = 0;
			}
		}
}

void Section::loadFaces()
{
	std::vector<GLfloat> faces;
	std::vector<GLuint> indices;

	for (int i = 0; i < SIDE; i++)
		for (int j = 0; j < HEIGHT; j++)
			for (int k = 0; k < SIDE; k++)
			{
				if (m_blocks.at(ivec3{ i, j, k }) == 0)
					continue;

				vec3 globalPos{ i + m_position.x * SIDE, j + m_position.y * HEIGHT, k + m_position.z * SIDE };

				if (!isInSection(ivec3{ i + 1, j, k }) || m_blocks.at(ivec3{ i + 1, j, k }) == 0)
					for (GLfloat c : getFace(globalPos, faceX1))
						faces.push_back(c);

				if (!isInSection(ivec3{ i - 1, j, k }) || m_blocks.at(ivec3{ i - 1, j, k }) == 0)
					for (GLfloat c : getFace(globalPos, faceX0))
						faces.push_back(c);

				if (!isInSection(ivec3{ i, j + 1, k }) || m_blocks.at(ivec3{ i, j + 1, k }) == 0)
					for (GLfloat c : getFace(globalPos, faceY1))
						faces.push_back(c);

				if (!isInSection(ivec3{ i, j - 1, k }) || m_blocks.at(ivec3{ i, j - 1, k }) == 0)
					for (GLfloat c : getFace(globalPos, faceY0))
						faces.push_back(c);

				if (!isInSection(ivec3{ i, j, k + 1 }) || m_blocks.at(ivec3{ i, j, k + 1 }) == 0)
					for (GLfloat c : getFace(globalPos, faceZ1))
						faces.push_back(c);

				if (!isInSection(ivec3{ i, j, k - 1 }) || m_blocks.at(ivec3{ i, j, k - 1 }) == 0)
					for (GLfloat c : getFace(globalPos, faceZ0))
						faces.push_back(c);
			}

	for (int i = 0; i < faces.size() / 20; ++i) // Each 4 segments
		for (GLuint index : rectIndices) // Add the indices with an offset of 4 * i 
			indices.push_back(index + 4 * i);

	indicesNb = indices.size();

	// Create and bind the VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// The VBO stores the vertices
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * faces.size(), faces.data(), GL_STREAM_DRAW);

	// The EBO stores the indices
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STREAM_DRAW);

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

void Section::load()
{
	loadBlocks();
	loadFaces();
}

void Section::render(Shader &shader, Texture2D &texture)
{
	shader.use();
	glActiveTexture(GL_TEXTURE0);
	texture.bind();

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indicesNb, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	assert(glGetError() == GL_NO_ERROR);
}

bool Section::isInSection(ivec3 block)
{
	return 0 <= block.x && block.x < SIDE && 0 <= block.y && block.y < HEIGHT && 0 <= block.z && block.z < SIDE;
}

std::array<GLfloat, 20> Section::getFace(glm::ivec3 pos, const std::array<GLfloat, 12>& face)
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

const std::array<GLfloat, 6> Section::rectIndices{
	0, 1, 2,
	2, 3, 0
};

const std::array<GLfloat, 8> Section::textureCoords{
	0.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f
};

const std::array<GLfloat, 12> Section::faceX0{
	0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f
};

const std::array<GLfloat, 12> Section::faceX1{
	1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 0.0f
};

const std::array<GLfloat, 12> Section::faceY0{
	0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f
};

const std::array<GLfloat, 12> Section::faceY1{
	1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 0.0f
};

const std::array<GLfloat, 12> Section::faceZ0{
	1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f
};

const std::array<GLfloat, 12> Section::faceZ1{
	0.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 0.0f, 1.0f
};