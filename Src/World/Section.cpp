#include "Section.h"

#include "Generator/Noise/OctavePerlin.h"
#include "Maths/Converter.h"
#include "Chunk.h"
#include "Util/DebugGL.h"
#include "ResManager/ResManager.h"
#include "Util/Logger.h"
#include "CubeData.h"

#include <iostream>

Section::Section(const Chunk* chunk, ivec3 position)
	: p_chunk{ chunk }, m_position{ position },
	m_blocks{ std::make_unique<BlockArray>() }
{ }

int dirToBin(ivec3 dir) {
	dir += 1;
	return dir.x + (dir.y << 2) + (dir.z << 4);
}

const Section* Section::findNeighboringSection(Dir3D::Dir dir, const NeighborChunks& neighbors) const {
	// If the 3D direction is horizontal, it matches one of the neighboring chunks
	const Chunk* neighborChunk = p_chunk;
	auto dirOpt = Dir3D::to_2D(dir);
	if (dirOpt.has_value())
		neighborChunk = neighbors[dirOpt.value()];
	if (neighborChunk == nullptr)
		return nullptr;
	const int neighborY = m_position.y + Dir3D::to_ivec3(dir).y;
	if (0 <= neighborY && neighborY < neighborChunk->getHeight())
		return &neighborChunk->getSection(neighborY);
	return nullptr;
}

std::tuple<std::vector<DefaultMesh::Vertex>, std::vector<WaterMesh::Vertex> > Section::findVisibleFaces(const NeighborChunks& neighbors) const {
	std::vector<DefaultMesh::Vertex> defaultVertices;
	std::vector<WaterMesh::Vertex> waterVertices;
	
	// The axes are indexed by y = 0, x = 1 and z = 2;
	const std::array<ivec3, 3> AXIS_ORDER{ {
		{ 1, 0, 2 }, // y axis
		{ 0, 1, 2 }, // x axis
		{ 2, 1, 0 }  // z axis
	} };
	const ivec3 SECTION_BOUNDS{ Const::SECTION_SIDE, Const::SECTION_HEIGHT, Const::SECTION_SIDE };

	for (Dir3D::Dir dir : Dir3D::all()) {
		const int axis = dir % 3; // Axis of the current direction

		// Done once per direction because quite costly
		const Section* neighboringSection = findNeighboringSection(dir, neighbors);
		
		// The order by which the axes are iterated on
		// order[0] is the first axis on which the iteration occurs
		// order[2] is the last axis on which the iteration occurs
		const ivec3 order = AXIS_ORDER[axis];

		ivec3 orderedBounds; // Bounds of the section, in the order of the axes
		for (int i = 0; i < 3; ++i)
			orderedBounds[i] = SECTION_BOUNDS[order[i]];

		ivec3 localPos; // Current position in the section coordinates
		for (localPos[order[0]] = 0; localPos[order[0]] < orderedBounds[0]; ++localPos[order[0]]) {
			for (localPos[order[1]] = 0; localPos[order[1]] < orderedBounds[1]; ++localPos[order[1]]) {
				addVisibleFacesOnLastAxis(defaultVertices, waterVertices, dir, localPos, order[2],
						orderedBounds[2], neighboringSection, neighbors);
			}
		}
	}
	return { defaultVertices, waterVertices };
}

void Section::addVisibleFacesOnLastAxis(
	std::vector<DefaultMesh::Vertex>& defaultVertices, std::vector<WaterMesh::Vertex>& waterVertices,
	Dir3D::Dir dir, ivec3 localPos, int indexOfLastAxis, int sizeOfLastAxis,
	const Section* neighboringSection, const NeighborChunks& neighbors) const {

	const ivec3 dirVec = Dir3D::to_ivec3(dir);
	const std::array<GLfloat, 4> noAO{ 1.f, 1.f, 1.f, 1.f };
	Block lastBlock{ BlockID::AIR }; // Last block we have iterated on
	std::array<GLfloat, 4> lastAO{ noAO }; // Ambient occlusion of the face we are currently extending
	int firstBlockIndex = -1; // Index on the last axis of the first block of the face we are creating

	for (localPos[indexOfLastAxis] = 0; localPos[indexOfLastAxis] < sizeOfLastAxis; ++localPos[indexOfLastAxis]) {
		Block currBlock{ BlockID::AIR }; // Current block of which we can see the face
		std::array<GLfloat, 4> currAO{ noAO };

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
				currAO = computeFaceAO(neighbors, dir, localPos);
			}
		}
		// Add the last face because we can't extend it if the current block is different. A merged
		// face carries a single ambient occlusion value per corner, so a change in AO also ends it.
		if (currBlock.id != lastBlock.id || currAO != lastAO) {
			addFace(defaultVertices, waterVertices, dir, localPos, firstBlockIndex, lastBlock, indexOfLastAxis, lastAO);
			firstBlockIndex = localPos[indexOfLastAxis];
			lastBlock = currBlock;
			lastAO = currAO;
		}
	}
	// Add the last face if at the end of last axis
	addFace(defaultVertices, waterVertices, dir, localPos, firstBlockIndex, lastBlock, indexOfLastAxis, lastAO);
}

void Section::addFace(std::vector<DefaultMesh::Vertex>& defaultVertices, std::vector<WaterMesh::Vertex>& waterVertices,
	Dir3D::Dir dir, ivec3 localPos, int firstBlockIndex, Block block, int indexOfLastAxis,
	const std::array<GLfloat, 4>& ao) const {

	BlockData::Category category = ResManager::blockDatas().get(block.id).getCategory();
	if (category != BlockData::AIR) {
		ivec3 negativeLastAxis{ 0, 0, 0 };
		negativeLastAxis[indexOfLastAxis] = -1;
		int length = localPos[indexOfLastAxis] - firstBlockIndex;
		ivec3 firstBlockGlobalPos{ Converter::sectionToGlobal(m_position) + localPos + negativeLastAxis * length };

		if (category == BlockData::DEFAULT || category == BlockData::SEMI_TRANSPARENT) {
			addDefaultFace(defaultVertices, dir, block, indexOfLastAxis, length, firstBlockGlobalPos, ao);
		}
		else if (category == BlockData::WATER) {
			addWaterFace(waterVertices, dir, indexOfLastAxis, length, firstBlockGlobalPos);
		}
	}
}

void Section::addDefaultFace(std::vector<DefaultMesh::Vertex>& defaultVertices, Dir3D::Dir dir, Block block,
	int indexOfLastAxis, int length, ivec3 firstBlockGlobalPos, const std::array<GLfloat, 4>& ao) const {

	GLuint texID = ResManager::blockDatas().get(block.id).getTexture(dir);
	for (int vtx = 0; vtx < 4; ++vtx) {
		vec3 currVtx = CubeData::dirToFace[dir][vtx];
		currVtx[indexOfLastAxis] *= length;

		vec2 tex = CubeData::faceCoords[vtx];
		if (indexOfLastAxis == 1) { // Extend the y axis if the last axis is the y axis
			tex.y *= length;
		} else {
			tex.x *= length;
		}
		defaultVertices.push_back({ currVtx + vec3(firstBlockGlobalPos), tex, Dir3D::to_ivec3(dir), texID, ao[vtx] });
	}
}

void Section::addWaterFace(std::vector<WaterMesh::Vertex>& waterVertices, Dir3D::Dir dir,
	int indexOfLastAxis, int length, ivec3 firstBlockGlobalPos) const {

	auto addWaterFaceInDir = [&indexOfLastAxis, &length, &waterVertices](Dir3D::Dir dir, ivec3 firstBlockGlobalPos) {
		for (int vtx = 0; vtx < 4; ++vtx) {
			vec3 currVtx = CubeData::dirToFace[dir][vtx];
			currVtx[indexOfLastAxis] *= length;
			waterVertices.push_back({ currVtx + vec3(firstBlockGlobalPos), Dir3D::to_ivec3(dir) });
		}
	};
	addWaterFaceInDir(dir, firstBlockGlobalPos);
	addWaterFaceInDir(Dir3D::opp(dir), firstBlockGlobalPos + Dir3D::to_ivec3(dir));
}

std::array<GLfloat, 4> Section::computeFaceAO(const NeighborChunks& neighbors, Dir3D::Dir dir, ivec3 localPos) const {
	const ivec3 n = Dir3D::to_ivec3(dir);
	// The axis the face points along, and the two axes spanning the face plane.
	const int normalAxis = (n.x != 0) ? 0 : (n.y != 0 ? 1 : 2);
	const int u = (normalAxis + 1) % 3;
	const int v = (normalAxis + 2) % 3;
	const ivec3 base = localPos + n; // One block out, into the layer that can occlude the face.

	// Brightness per occlusion level: 0 = corner fully tucked between two blocks, 3 = open.
	static const std::array<GLfloat, 4> levelBrightness{ 0.5f, 0.7f, 0.85f, 1.f };

	auto isSolid = [&](ivec3 pos) {
		return ResManager::blockDatas().get(getBlockForAO(neighbors, pos).id).isOpaque();
	};

	std::array<GLfloat, 4> ao;
	for (int vtx = 0; vtx < 4; ++vtx) {
		const vec3 corner = CubeData::dirToFace[dir][vtx];
		ivec3 du{ 0, 0, 0 }; du[u] = (corner[u] > 0.5f) ? 1 : -1;
		ivec3 dv{ 0, 0, 0 }; dv[v] = (corner[v] > 0.5f) ? 1 : -1;

		const int side1 = isSolid(base + du) ? 1 : 0;
		const int side2 = isSolid(base + dv) ? 1 : 0;
		const int corn = isSolid(base + du + dv) ? 1 : 0;
		// Two side blocks fully close the corner, hiding whatever is diagonally behind it.
		const int level = (side1 && side2) ? 0 : (3 - (side1 + side2 + corn));
		ao[vtx] = levelBrightness[level];
	}
	return ao;
}

Block Section::getBlockForAO(const NeighborChunks& neighbors, ivec3 localPos) const {
	const Block air{ BlockID::AIR };
	if (isInSection(localPos))
		return getBlock(localPos);

	const Chunk* chunk = p_chunk;
	// Horizontal crossing into a cardinal neighbor chunk. A diagonal crossing (both axes out at
	// once, only at a chunk's vertical corner edges) would need a neighbor we do not snapshot, so
	// it counts as air; the resulting AO error is confined to those rare corner columns.
	if (localPos.x < 0 || localPos.x >= Const::SECTION_SIDE ||
		localPos.z < 0 || localPos.z >= Const::SECTION_SIDE) {
		const int dx = localPos.x < 0 ? -1 : (localPos.x >= Const::SECTION_SIDE ? 1 : 0);
		const int dz = localPos.z < 0 ? -1 : (localPos.z >= Const::SECTION_SIDE ? 1 : 0);
		if (dx != 0 && dz != 0)
			return air;
		const Dir2D::Dir dir = dx == 1 ? Dir2D::FRONT : dx == -1 ? Dir2D::BACK
							 : dz == 1 ? Dir2D::RIGHT : Dir2D::LEFT;
		chunk = neighbors[dir];
		if (chunk == nullptr)
			return air;
		localPos.x = (localPos.x + Const::SECTION_SIDE) % Const::SECTION_SIDE;
		localPos.z = (localPos.z + Const::SECTION_SIDE) % Const::SECTION_SIDE;
	}

	// Vertical crossing into another stacked section of the resolved chunk.
	const int globalY = m_position.y * Const::SECTION_HEIGHT + localPos.y;
	if (globalY < 0 || globalY >= chunk->getHeight() * Const::SECTION_HEIGHT)
		return air;
	return chunk->getBlock({ localPos.x, globalY, localPos.z });
}

std::vector<GLuint> getIndices(int size) {
	std::vector<GLuint> indices;
	for (int faceIndex = 0; faceIndex < size; ++faceIndex) // Each face (4 vertices)
		for (GLuint rectIndex : CubeData::faceElementIndices) // Add the indices with an offset of 4 * faceIndex
			indices.push_back(4 * faceIndex + rectIndex);
	return indices;
}

void Section::loadMesh(const NeighborChunks& neighbors) {
	// CPU only: build the vertex data. Runs on a worker thread, so it must not touch OpenGL.
	tie(m_nextDefaultVertices, m_nextWaterVertices) = findVisibleFaces(neighbors);
}

void Section::uploadMesh() {
	// All OpenGL for the mesh happens here, on the main thread
	nextDefaultMesh.loadBuffers(m_nextDefaultVertices, getIndices((int)m_nextDefaultVertices.size() / 4));
	nextWaterMesh.loadBuffers(m_nextWaterVertices, getIndices((int)m_nextWaterVertices.size() / 4));
	nextDefaultMesh.loadVAOs();
	nextWaterMesh.loadVAOs();

	activeDefaultMesh = std::move(nextDefaultMesh);
	activeWaterMesh = std::move(nextWaterMesh);

	// Free memory
	m_nextDefaultVertices = {};
	m_nextWaterVertices = {};

	Debug::glCheckError();
}

void Section::releaseMesh() {
	activeDefaultMesh.release();
	activeWaterMesh.release();
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