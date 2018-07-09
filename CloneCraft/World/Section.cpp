#include "Section.h"

#include "OctavePerlin.h"
#include "Converter.h"
#include "ChunkMap.h"
#include "Debug.h"

#include <iostream>

template<typename T, int S>
using arr = std::array<T, S>;

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
				int block = p_chunk->getChunkGenerator().getBlock(pos + Converter::sectionToGlobal(m_position));
				if (block != 0)
					empty = false;
				m_blocks.at(pos) = block;
			}
}

int dirToBin(ivec3 dir) {
	dir += 1;
	return dir.x + (dir.y << 2) + (dir.z << 4);
}

std::vector<Face> Section::findFaces() {
	std::vector<Face> faces;

	const arr<ivec3, 3> axeOrder{ {
		{ 1, 0, 2 }, // Y axe
		{ 0, 1, 2 }, // X axe
		{ 2, 1, 0 }  // Z axe
	} };

	for (int dir = 0; dir < Dir3D::SIZE; ++dir) {
		const ivec3 dirPos = Dir3D::find(static_cast<Dir3D::Dir>(dir));
		const int axe = dir % 3; // Axe of the current direction
		const ivec3 neighbourPos = m_position + dirPos; // Position of the neighbour section
		const Section* neighbour = (neighbourPos.y < 0 || neighbourPos.y >= Const::CHUNK_NB_SECTIONS) 
			? nullptr : &(p_chunkMap->getSection(neighbourPos)); // Neighbour section (nullptr if does not exist)
		const arr<vec3, 4> face = dirToFace[dir]; // Face associated with the current direction
		const ivec3 order = axeOrder[axe]; // For the current axe, order[i] = j means that the axe i is associated with the value j

		const ivec3 MAXS{ Const::SECTION_SIDE, Const::SECTION_HEIGHT, Const::SECTION_SIDE };
		ivec3 maxsAxe; // Maximum value of each axe by index
		for (int i = 0; i < 3; ++i)
			maxsAxe[i] = MAXS[axeOrder[axe][i]];

		int indexOfLastAxe; // Index of the axe on which we iterate in the last loop (0 = x, 1 = y, 2 = z)
		for (int i = 0; i < 3; ++i) {
			if (order[i] == 2)
				indexOfLastAxe = i;
		}
		ivec3 oppositeOfLastAxe{ 0, 0, 0 };
		oppositeOfLastAxe[indexOfLastAxe] = -1;

		ivec3 abstPos;
		for (abstPos.x = 0; abstPos.x < maxsAxe.x; ++abstPos) {
			for (abstPos.y = 0; abstPos.y < maxsAxe.y; ++abstPos.y) {
				int lastBlock = 0; // Last block we have iterated on
				int firstBlockPos = -1; // Index of the last axe containing the first block of the iteration

				auto addFace = [&](int c, ivec3 localPos) {
					if (lastBlock != 0) {
						int length = c - firstBlockPos; // Length of the face
						const ivec3 firstBlockGlobalPos{ Converter::sectionToGlobal(m_position) + localPos + oppositeOfLastAxe * length };
						for (int vtx = 0; vtx < 4; ++vtx) {
							vec3 currFace = face[vtx];
							currFace[indexOfLastAxe] *= length;
							// Multiply coordinate x of the texture (depends on the vertices of the face)
							GLuint texNorm = textureCoords[vtx].x * length + (textureCoords[vtx].y << 8)
								+ (dirToBin(dirPos) << 16);
							faces.push_back({ currFace + vec3(firstBlockGlobalPos), texNorm });
						}
					}
				};

				for (abstPos.z = 0; abstPos.z < maxsAxe.z; ++abstPos.z) {
					int currBlock = 0; // Current block of which we can see the face
					// Position in the section with correct x, y and z coordinates
					const ivec3 localPos{ abstPos[order.x], abstPos[order.y], abstPos[order.z] };
					const int block = getBlock(localPos);
					if (block != 0) { // If the block is air, its face will be air anyway
						const ivec3 globalPos{ Converter::sectionToGlobal(m_position) + localPos };
						const ivec3 localFacePos{ localPos + dirPos };
						const ivec3 globalFacePos = { globalPos + dirPos };
						int blockFace; // Face of the block in the direction of the face
						if (globalFacePos.y < 0 || globalFacePos.y >= Const::CHUNK_HEIGHT) {
							blockFace = 0;
						} else {
							// The block can only be in the current section or the neighbour section
							blockFace = isInSection(localFacePos) ? this->getBlock(localFacePos)
								: neighbour->getBlock(Converter::globalToInnerSection(localFacePos));
						}
						if (blockFace == 0) { // The face is visible only if the block in the direction of the face is air
							currBlock = block;
						}
					}
					if (currBlock != lastBlock) { // We can't extend the last face if the current face is different
						addFace(abstPos.z, localPos);
						firstBlockPos = abstPos.z;
						lastBlock = currBlock;
					}
				}
				const ivec3 localPos{ abstPos[order.x], abstPos[order.y], abstPos[order.z] };
				// Add the last face if at the end of the section
				addFace(maxsAxe.z, localPos);
			}
		}
	}
	return faces;
}

void Section::loadFaces() {
	if (empty)
		return;

	std::vector<Face> faces = findFaces();

	std::vector<GLuint> indices;
	for (int faceIndex = 0; faceIndex < (int)faces.size() / 4; ++faceIndex) // Each face (4 vertices)
		for (GLuint rectIndex : rectIndices) // Add the indices with an offset of 4 * faceIndex
			indices.push_back(4 * faceIndex + rectIndex);

	indicesNb = indices.size();
	if (indicesNb != 0) {
		// The VBO stores the vertices
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Face) * faces.size(), faces.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// The EBO stores the indices
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		Debug::glCheckError();
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
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Face), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(Face), (GLvoid*)sizeof(vec3));
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
	}
}

GLuint Section::getBlock(ivec3 pos) const {
	return m_blocks.at(pos);
}

bool Section::isInSection(ivec3 pos) {
	return 0 <= pos.x && pos.x < Const::SECTION_SIDE && 0 <= pos.y && pos.y < Const::SECTION_HEIGHT && 
			0 <= pos.z && pos.z < Const::SECTION_SIDE;
}

GLuint Section::getNearBlock(ivec3 pos) {
	if (isInSection(pos)) {
		return m_blocks.at(pos);
	} else {
		ivec3 globalPos{ pos + Converter::sectionToGlobal(m_position) };
		return p_chunkMap->getBlock(globalPos);
	}
}

const arr<GLuint, 6> Section::rectIndices{
	0, 1, 2,
	2, 3, 0
};

const arr<ivec2, 4> Section::textureCoords{{
	{ 0, 0 },
	{ 0, 1 },
	{ 1, 1 },
	{ 1, 0 }
} };

const arr<arr<vec3, 4>, Dir3D::SIZE> Section::dirToFace = []() {

	const arr<vec3, 4> faceX0{ {
		{ 0, 0, 0 },
		{ 0, 1, 0 },
		{ 0, 1, 1 },
		{ 0, 0, 1 }
	} };

	const arr<vec3, 4> faceX1{ {
		{ 1, 0, 1 },
		{ 1, 1, 1 },
		{ 1, 1, 0 },
		{ 1, 0, 0 }
	} };

	const arr<vec3, 4> faceY0{ {
		{ 0, 0, 1 },
		{ 1, 0, 1 },
		{ 1, 0, 0 },
		{ 0, 0, 0 }
	} };

	const arr<vec3, 4> faceY1{ {
		{ 1, 1, 1 },
		{ 0, 1, 1 },
		{ 0, 1, 0 },
		{ 1, 1, 0 }
	} };

	const arr<vec3, 4> faceZ0{{
		{ 1, 0, 0 },
		{ 1, 1, 0 },
		{ 0, 1, 0 },
		{ 0, 0, 0 }
	}};

	const arr<vec3, 4> faceZ1{ {
		{ 0, 0, 1 },
		{ 0, 1, 1 },
		{ 1, 1, 1 },
		{ 1, 0, 1 }
	} };

	return arr<arr<vec3, 4>, Dir3D::SIZE>{ faceY1, faceX1, faceZ1, faceY0, faceX0, faceZ0 };
}();