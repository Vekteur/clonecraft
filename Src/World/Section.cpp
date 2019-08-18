#include "Section.h"

#include "Generator/Noise/OctavePerlin.h"
#include "Maths/Converter.h"
#include "ChunkMap.h"
#include "Util/Debug.h"
#include "ResManager/ResManager.h"
#include "Util/Logger.h"
#include "CubeData.h"

#include <iostream>

Section::Section(const ChunkMap* chunkMap, const Chunk* chunk, ivec3 position)
	: p_chunkMap{ chunkMap }, p_chunk{ chunk }, m_position{ position }, 
	m_blocks{ std::make_unique<BlockArray>() }
{ }

int dirToBin(ivec3 dir) {
	dir += 1;
	return dir.x + (dir.y << 2) + (dir.z << 4);
}

const Section* Section::findNeighboringSection(Dir3D::Dir dir) const {
	const ivec3 neighbourPos = m_position + Dir3D::to_ivec3(dir); // Position of the neighbour section
	const Chunk& neighbourChunk = p_chunkMap->getChunk(Converter::to2D(neighbourPos));
	const Section* neighbourSection = nullptr;
	if (0 <= neighbourPos.y && neighbourPos.y < neighbourChunk.getHeight())
		neighbourSection = &neighbourChunk.getSection(neighbourPos.y);
	return neighbourSection;
}

std::tuple<std::vector<DefaultMesh::Vertex>, std::vector<WaterMesh::Vertex> > Section::findVisibleFaces() const {
	std::vector<DefaultMesh::Vertex> defaultVertices;
	std::vector<WaterMesh::Vertex> waterVertices;
	
	// The axes are indexed by y = 0, x = 1 and z = 2;
	const std::array<ivec3, 3> AXE_ORDER{ {
		{ 1, 0, 2 }, // y axe
		{ 0, 1, 2 }, // x axe
		{ 2, 1, 0 }  // z axe
	} };
	const ivec3 SECTION_BOUNDS{ Const::SECTION_SIDE, Const::SECTION_HEIGHT, Const::SECTION_SIDE };

	for (Dir3D::Dir dir : Dir3D::all()) {
		const int axe = dir % 3; // Axe of the current direction

		// Done once per direction because quite costly
		const Section* neighboringSection = findNeighboringSection(dir);
		
		// The order by which the axes are iterated on
		// order[0] is the first axe on which the iteration occurs
		// order[2] is the last axe on which the iteration occurs
		const ivec3 order = AXE_ORDER[axe];

		ivec3 orderedBounds; // Bounds of the section, in the order of the axes
		for (int i = 0; i < 3; ++i)
			orderedBounds[i] = SECTION_BOUNDS[order[i]];

		ivec3 localPos; // Current position in the section coordinates
		for (localPos[order[0]] = 0; localPos[order[0]] < orderedBounds[0]; ++localPos[order[0]]) {
			for (localPos[order[1]] = 0; localPos[order[1]] < orderedBounds[1]; ++localPos[order[1]]) {
				addVisibleFacesOnLastAxe(defaultVertices, waterVertices, dir, localPos, order[2],
						orderedBounds[2], neighboringSection);
			}
		}
	}
	return { defaultVertices, waterVertices };
}

void Section::addVisibleFacesOnLastAxe(
	std::vector<DefaultMesh::Vertex>& defaultVertices, std::vector<WaterMesh::Vertex>& waterVertices,
	Dir3D::Dir dir, ivec3 localPos, int indexOfLastAxe, int sizeOfLastAxe,
	const Section* neighboringSection) const {

	const ivec3 dirVec = Dir3D::to_ivec3(dir);
	Block lastBlock{ BlockID::AIR }; // Last block we have iterated on
	int firstBlockIndex = -1; // Index on the last axe of the first block of the face we are creating

	for (localPos[indexOfLastAxe] = 0; localPos[indexOfLastAxe] < sizeOfLastAxe; ++localPos[indexOfLastAxe]) {
		Block currBlock{ BlockID::AIR }; // Current block of which we can see the face

		const Block block = this->getBlock(localPos);
		// We can pass the air block for efficiency because its face will be invisible anyway
		if (block.id != +BlockID::AIR) {
			const ivec3 globalPos{ Converter::sectionToGlobal(m_position) + localPos };
			// The neighboring position in the current direction
			const ivec3 localNeighPos{ localPos + dirVec };
			const int globalNeighHeight = globalPos.y + dirVec.y;
			Block neighBlock{ BlockID::AIR };
			// The blocks vertically outside the chunk are air block
			if (0 <= globalNeighHeight && globalNeighHeight < p_chunk->getHeight() * Const::SECTION_HEIGHT) {
				// The block can only be in the current section or in the neighboring section
				if (isInSection(localNeighPos)) {
					neighBlock = this->getBlock(localNeighPos);
				}
				else if (neighboringSection != nullptr) {
					neighBlock = neighboringSection->getBlock(Converter::globalToInnerSection(localNeighPos));
				}
			}
			// The face is visible only if the block in the direction of the face is transparent
			// and if this block is different from the current block
			if (!ResManager::blockDatas().get(neighBlock.id).isOpaque() && neighBlock.id != block.id) {
				currBlock = block;
			}
		}
		// Add the last face because we can't extend it if the current block is different
		if (currBlock.id != lastBlock.id) {
			addFace(defaultVertices, waterVertices, dir, localPos, firstBlockIndex, lastBlock, indexOfLastAxe);
			firstBlockIndex = localPos[indexOfLastAxe];
			lastBlock = currBlock;
		}
	}
	// Add the last face if at the end of last axe
	addFace(defaultVertices, waterVertices, dir, localPos, firstBlockIndex, lastBlock, indexOfLastAxe);
}

void Section::addFace(std::vector<DefaultMesh::Vertex>& defaultVertices, std::vector<WaterMesh::Vertex>& waterVertices,
	Dir3D::Dir dir, ivec3 localPos, int firstBlockIndex, Block block, int indexOfLastAxe) const {

	BlockData::Category category = ResManager::blockDatas().get(block.id).getCategory();
	if (category != BlockData::AIR) {
		ivec3 negativeLastAxe{ 0, 0, 0 };
		negativeLastAxe[indexOfLastAxe] = -1;
		int length = localPos[indexOfLastAxe] - firstBlockIndex;
		ivec3 firstBlockGlobalPos{ Converter::sectionToGlobal(m_position) + localPos + negativeLastAxe * length };

		if (category == BlockData::DEFAULT || category == BlockData::SEMI_TRANSPARENT) {
			addDefaultFace(defaultVertices, dir, block, indexOfLastAxe, length, firstBlockGlobalPos);
		}
		else if (category == BlockData::WATER) {
			addWaterFace(waterVertices, dir, indexOfLastAxe, length, firstBlockGlobalPos);
		}
	}
}

void Section::addDefaultFace(std::vector<DefaultMesh::Vertex>& defaultVertices, Dir3D::Dir dir, Block block,
	int indexOfLastAxe, int length, ivec3 firstBlockGlobalPos) const {

	GLuint texID = ResManager::blockDatas().get(block.id).getTexture(dir);
	for (int vtx = 0; vtx < 4; ++vtx) {
		vec3 currVtx = CubeData::dirToFace[dir][vtx];
		currVtx[indexOfLastAxe] *= length;

		vec2 tex = CubeData::faceCoords[vtx];
		if (indexOfLastAxe == 1) { // Extend the y axe if the last axe is the y axe
			tex.y *= length;
		} else {
			tex.x *= length;
		}
		defaultVertices.push_back({ currVtx + vec3(firstBlockGlobalPos), tex, Dir3D::to_ivec3(dir), texID });
	}
}

void Section::addWaterFace(std::vector<WaterMesh::Vertex>& waterVertices, Dir3D::Dir dir,
	int indexOfLastAxe, int length, ivec3 firstBlockGlobalPos) const {

	auto addWaterFaceInDir = [&indexOfLastAxe, &length, &waterVertices](Dir3D::Dir dir, ivec3 firstBlockGlobalPos) {
		for (int vtx = 0; vtx < 4; ++vtx) {
			vec3 currVtx = CubeData::dirToFace[dir][vtx];
			currVtx[indexOfLastAxe] *= length;
			waterVertices.push_back({ currVtx + vec3(firstBlockGlobalPos), Dir3D::to_ivec3(dir) });
		}
	};
	addWaterFaceInDir(dir, firstBlockGlobalPos);
	addWaterFaceInDir(Dir3D::opp(dir), firstBlockGlobalPos + Dir3D::to_ivec3(dir));
}

std::vector<GLuint> getIndices(int size) {
	std::vector<GLuint> indices;
	for (int faceIndex = 0; faceIndex < size; ++faceIndex) // Each face (4 vertices)
		for (GLuint rectIndex : CubeData::faceElementIndices) // Add the indices with an offset of 4 * faceIndex
			indices.push_back(4 * faceIndex + rectIndex);
	return indices;
}

void Section::loadFaces() {
	std::vector<DefaultMesh::Vertex> defaultVertices;
	std::vector<WaterMesh::Vertex> waterVertices;
	tie(defaultVertices, waterVertices) = findVisibleFaces();
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
	m_blocks->at(pos) = block;
}

Block Section::getBlock(ivec3 pos) const {
	return m_blocks->at(pos);
}

bool Section::isInSection(ivec3 globalPos) const {
	return 0 <= globalPos.x && globalPos.x < Const::SECTION_SIDE && 
			0 <= globalPos.y && globalPos.y < Const::SECTION_HEIGHT && 
			0 <= globalPos.z && globalPos.z < Const::SECTION_SIDE;
}