#include "Section.h"

#include "OctavePerlin.h"
#include "Converter.h"
#include "ChunkMap.h"
#include "Debug.h"
#include "ResManager.h"
#include "Logger.h"
#include "CubeData.h"

#include <iostream>

template<typename T, int S>
using arr = std::array<T, S>;

template<typename T>
using vec = std::vector<T>;

Section::Section(ChunkMap* const chunkMap, Chunk* const chunk, ivec3 position)
	: p_chunkMap{ chunkMap }, p_chunk{ chunk }, m_position{ position },
	m_blocks{ ivec3{ Const::SECTION_SIDE, Const::SECTION_HEIGHT, Const::SECTION_SIDE } }
{ }

void Section::loadBlocks() {
	for (int x = 0; x < Const::SECTION_SIDE; ++x)
		for (int z = 0; z < Const::SECTION_SIDE; ++z)
			for (int y = 0; y < Const::SECTION_HEIGHT; ++y) {
				ivec3 pos{ x, y, z };
				Block block = p_chunk->getChunkGenerator().getBlock(pos + Converter::sectionToGlobal(m_position));
				if (block.id != static_cast<ID>(ID::AIR))
					empty = false;
				m_blocks.at(pos) = block;
			}
}

int dirToBin(ivec3 dir) {
	dir += 1;
	return dir.x + (dir.y << 2) + (dir.z << 4);
}

std::tuple<vec<DefaultMesh::Vertex>, vec<WaterMesh::Vertex> > Section::findFaces() {
	vec<DefaultMesh::Vertex> defaultVertices;
	vec<WaterMesh::Vertex> waterVertices;

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
		const arr<vec3, 4> face = CubeData::dirToFace[dir]; // Face associated with the current direction
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
				Block lastBlock{ ID::AIR }; // Last block we have iterated on
				int firstBlockPos = -1; // Index of the last axe containing the first block of the iteration

				auto addFace = [&](int currBlockPos, ivec3 localPos) {
					BlockData::Category category = ResManager::blockDatas().get(lastBlock.id).getCategory();
					if (category != BlockData::AIR) {
						int length = currBlockPos - firstBlockPos; // Length of the face
						const ivec3 firstBlockGlobalPos{ Converter::sectionToGlobal(m_position) + localPos + oppositeOfLastAxe * length };
						
						if (category == BlockData::DEFAULT) {
							GLuint texID = ResManager::blockDatas().get(lastBlock.id).getTexture(static_cast<Dir3D::Dir>(dir));
							for (int vtx = 0; vtx < 4; ++vtx) {
								vec3 currVtx = face[vtx];
								currVtx[indexOfLastAxe] *= length;
								// Multiply coordinate x of the texture (depends on the vertices of the face)
								vec2 tex = { CubeData::faceCoords[vtx].x * length, CubeData::faceCoords[vtx].y };
								defaultVertices.push_back({ currVtx + vec3(firstBlockGlobalPos), tex, dirPos, texID });
							}
						} else if (category == BlockData::WATER) {
							for (int vtx = 0; vtx < 4; ++vtx) {
								vec3 currVtx = face[vtx];
								currVtx[indexOfLastAxe] *= length;
								// Multiply coordinate x of the texture (depends on the vertices of the face)
								vec2 tex = { CubeData::faceCoords[vtx].x * length, CubeData::faceCoords[vtx].y };
								waterVertices.push_back({ currVtx + vec3(firstBlockGlobalPos), tex, dirPos });
							}
						}
					}
				};

				for (abstPos.z = 0; abstPos.z < maxsAxe.z; ++abstPos.z) {
					Block currBlock{ ID::AIR }; // Current block of which we can see the face
					// Position in the section with correct x, y and z coordinates
					const ivec3 localPos{ abstPos[order.x], abstPos[order.y], abstPos[order.z] };
					const Block block = getBlock(localPos);
					if (block.id != +ID::AIR) { // If the block is air, its face will be air anyway
						const ivec3 globalPos{ Converter::sectionToGlobal(m_position) + localPos };
						const ivec3 localFacePos{ localPos + dirPos };
						const ivec3 globalFacePos = { globalPos + dirPos };
						Block blockFace{ ID::AIR }; // Face of the block in the direction of the face
						if (globalFacePos.y < 0 || globalFacePos.y >= Const::CHUNK_HEIGHT) {
							blockFace = Block{ ID::AIR };
						} else {
							// The block can only be in the current section or the neighbour section
							blockFace = isInSection(localFacePos) ? this->getBlock(localFacePos)
								: neighbour->getBlock(Converter::globalToInnerSection(localFacePos));
						}
						// The face is visible only if the block in the direction of the face is transparent
						// and if this block is different than the current block
						if (!ResManager::blockDatas().get(blockFace.id).isOpaque() && blockFace.id != block.id) { 
							currBlock = block;
						}
					}
					if (currBlock.id != lastBlock.id) { // We can't extend the last face if the current face is different
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
	return { defaultVertices, waterVertices };
}

vec<GLuint> getIndices(int size) {
	vec<GLuint> indices;
	for (int faceIndex = 0; faceIndex < size; ++faceIndex) // Each face (4 vertices)
		for (GLuint rectIndex : CubeData::faceElementIndices) // Add the indices with an offset of 4 * faceIndex
			indices.push_back(4 * faceIndex + rectIndex);
	return indices;
}

void Section::loadFaces() {
	if (empty)
		return;

	vec<DefaultMesh::Vertex> defaultVertices;
	vec<WaterMesh::Vertex> waterVertices;
	tie(defaultVertices, waterVertices) = findFaces();
	nextDefaultMesh.loadBuffers(defaultVertices, getIndices((int)defaultVertices.size() / 4));
	nextWaterMesh.loadBuffers(waterVertices, getIndices((int)waterVertices.size() / 4));

	Debug::glCheckError();
}

void Section::loadVAOs() {
	nextDefaultMesh.loadVAOs();
	nextWaterMesh.loadVAOs();
	
	activeDefaultMesh = std::move(nextDefaultMesh);
	activeWaterMesh = std::move(nextWaterMesh);
}

void Section::unloadVAOs() {
	activeDefaultMesh.unloadVAOs();
	activeWaterMesh.unloadVAOs();
}

void Section::render(const DefaultRenderer &defaultRenderer) const {
	defaultRenderer.render(activeDefaultMesh);
}

void Section::render(const WaterRenderer& waterRenderer) const {
	waterRenderer.render(activeWaterMesh);
}

ivec3 Section::getPosition() const {
	return m_position;
}

void Section::setBlock(ivec3 pos, Block block) {
	m_blocks.at(pos) = block;
	if (block.id != static_cast<ID>(ID::AIR))
		empty = false;
}

Block Section::getBlock(ivec3 pos) const {
	return m_blocks.at(pos);
}

bool Section::isInSection(ivec3 pos) {
	return 0 <= pos.x && pos.x < Const::SECTION_SIDE && 0 <= pos.y && pos.y < Const::SECTION_HEIGHT && 
			0 <= pos.z && pos.z < Const::SECTION_SIDE;
}