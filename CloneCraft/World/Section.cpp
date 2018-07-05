#include "Section.h"

#include "OctavePerlin.h"
#include "Converter.h"
#include "ChunkMap.h"
#include "Debug.h"

#include <iostream>

Section::Section(ChunkMap* const chunkMap, Chunk* const chunk, ivec3 position)
		: p_chunkMap{ chunkMap }, p_chunk{ chunk }, m_position{ position }, 
		m_blocks{ ivec3{ Const::SECTION_SIDE, Const::SECTION_HEIGHT, Const::SECTION_SIDE } } {
}

Section::~Section() {
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void Section::loadBlocks() {
	for (int x = 0; x < Const::SECTION_SIDE; ++x)
		for (int z = 0; z < Const::SECTION_SIDE; ++z)
			for (int y = 0; y < Const::SECTION_HEIGHT; ++y) {
				ivec3 pos{ x, y, z };
				m_blocks.at(pos) = p_chunk->getChunkGenerator().getBlock(pos + Converter::sectionToGlobal(m_position));
			}
}

void Section::loadFaces() {
	std::vector<GLfloat> faces;
	std::vector<GLuint> indices;

	for (int x = 0; x < Const::SECTION_SIDE; ++x)
		for (int y = 0; y < Const::SECTION_HEIGHT; ++y)
			for (int z = 0; z < Const::SECTION_SIDE; ++z) {
				ivec3 pos{ x, y, z };
				if (m_blocks.at(pos) == 0)
					continue;

				ivec3 globalPos{ Converter::sectionToGlobal(m_position) + pos };

				for (int dir = 0; dir < Dir3D::SIZE; ++dir) {
					if (isAir(pos + Dir3D::find(static_cast<Dir3D::Dir>(dir))))
						for (GLfloat c : getFace(globalPos, dirToFace[dir]))
							faces.push_back(c);
				}
			}

	for (int i = 0; i < (int)(faces.size() / 20); ++i) // Each 4 segments
		for (GLuint index : rectIndices) // Add the indices with an offset of 4 * i
			indices.push_back(index + 4 * i);

	indicesNb = indices.size();

	if (indicesNb != 0) {
		// The VBO stores the vertices
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * faces.size(), faces.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// The EBO stores the indices
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

void Section::loadVAOs() {
	if (indicesNb != 0) {
		// Create and bind the VAO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		// Bind the buffers
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		// Attributes of the VAO
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		// Unbind all
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		// Unbind the EBO after unbinding the VAO else the EBO will be removed from the VAO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

void Section::unloadVAOs() {
	if (indicesNb != 0) {
		glDeleteVertexArrays(1, &VAO);
	}
}

void Section::render(Shader &shader, Texture2D &texture) const {
	if (indicesNb != 0) {
		// Activate shader and texture to draw the object
		shader.use();
		glActiveTexture(GL_TEXTURE0);
		texture.bind();

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indicesNb, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		Debug::glCheckError();
	}
}

int Section::getBlock(ivec3 pos) const {
	return m_blocks.at(pos);
}

bool Section::isInSection(ivec3 pos) {
	return 0 <= pos.x && pos.x < Const::SECTION_SIDE && 0 <= pos.y && pos.y < Const::SECTION_HEIGHT && 0 <= pos.z && pos.z < Const::SECTION_SIDE;
}

bool Section::isAir(ivec3 pos) {
	if (isInSection(pos))
		return m_blocks.at(pos) == 0;
	else {
		ivec3 globalPos{ pos + Converter::sectionToGlobal(m_position) };

		if (globalPos.y < 0 || globalPos.y >= Const::CHUNK_HEIGHT)
			return true;

		return p_chunkMap->getSection(Converter::globalToSection(globalPos)).getBlock(
			ivec3{ posMod(pos.x, Const::SECTION_SIDE), posMod(pos.y, Const::SECTION_HEIGHT), posMod(pos.z, Const::SECTION_SIDE) }) == 0;
	}
}

std::array<GLfloat, 20> Section::getFace(glm::ivec3 pos, const std::array<GLfloat, 12>& face) {
	std::array<GLfloat, 20> finalFace;
	// Each vertex
	for (int i = 0; i < 4; ++i) {
		// Compute coordinates of the vertex
		for (int j = 0; j < 3; ++j)
			finalFace[5 * i + j] = face[3 * i + j] + pos[j];
		// Compute texture coordinates of the vertex
		for (int j = 0; j < 2; ++j)
			finalFace[5 * i + (j + 3)] = textureCoords[2 * i + j];
	}
	return finalFace;
}

const std::array<GLuint, 6> Section::rectIndices{
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

const std::array<std::array<GLfloat, 12>, Dir3D::SIZE> Section::dirToFace{
	Section::faceX1, Section::faceY1, Section::faceZ1, Section::faceX0, Section::faceY0, Section::faceZ0
};