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
	// If the 3D direction is horizontal, it matches one of the neighboring chunks.
	const ivec3 v = Dir3D::to_ivec3(dir);
	const Chunk* neighborChunk = neighbors.at(v.x, v.z);
	if (neighborChunk == nullptr)
		return nullptr;
	// A missing section (above/below the populated range, or a sparse gap) reads as air.
	return neighborChunk->tryGetSection(m_position.y + v.y);
}

std::tuple<std::vector<DefaultMesh::Vertex>, std::vector<WaterMesh::Vertex>, std::vector<LavaMesh::Vertex> > Section::findVisibleFaces(const NeighborChunks& neighbors) const {
	std::vector<DefaultMesh::Vertex> defaultVertices;
	std::vector<WaterMesh::Vertex> waterVertices;
	std::vector<LavaMesh::Vertex> lavaVertices;
	
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
				addVisibleFacesOnLastAxis(defaultVertices, waterVertices, lavaVertices, dir, localPos, order[2],
						orderedBounds[2], neighboringSection, neighbors);
			}
		}
	}
	return { defaultVertices, waterVertices, lavaVertices };
}

void Section::addVisibleFacesOnLastAxis(
	std::vector<DefaultMesh::Vertex>& defaultVertices, std::vector<WaterMesh::Vertex>& waterVertices,
	std::vector<LavaMesh::Vertex>& lavaVertices,
	Dir3D::Dir dir, ivec3 localPos, int indexOfLastAxis, int sizeOfLastAxis,
	const Section* neighboringSection, const NeighborChunks& neighbors) const {

	const ivec3 dirVec = Dir3D::to_ivec3(dir);
	const std::array<GLfloat, 4> noAO{ 1.f, 1.f, 1.f, 1.f };
	// Full sky, no block light at all four corners: plain daylight.
	const std::array<vec2, 4> fullLight{ { { 1.f, 0.f }, { 1.f, 0.f }, { 1.f, 0.f }, { 1.f, 0.f } } };
	Block lastBlock{ BlockID::AIR }; // Last block we have iterated on
	std::array<GLfloat, 4> lastAO{ noAO }; // Ambient occlusion of the face we are currently extending
	std::array<vec2, 4> lastLight{ fullLight }; // Per-corner light of the face we are currently extending
	int firstBlockIndex = -1; // Index on the last axis of the first block of the face we are creating

	for (localPos[indexOfLastAxis] = 0; localPos[indexOfLastAxis] < sizeOfLastAxis; ++localPos[indexOfLastAxis]) {
		Block currBlock{ BlockID::AIR }; // Current block of which we can see the face
		std::array<GLfloat, 4> currAO{ noAO };
		std::array<vec2, 4> currLight{ fullLight };

		const Block block = this->getBlock(localPos);
		// We can skip the air block for efficiency because its face will be invisible anyway
		if (block.id != +BlockID::AIR) {
			// The neighboring position in the current direction
			const ivec3 localNeighPos{ localPos + dirVec };
			Block neighBlock{ BlockID::AIR };
			// The block can only be in the current section or in the neighboring section; a missing
			// neighboring section is air, so faces on the world's vertical edges stay visible.
			if (isInSection(localNeighPos)) {
				neighBlock = this->getBlock(localNeighPos);
			}
			else if (neighboringSection != nullptr) {
				neighBlock = neighboringSection->getBlock(Converter::globalToInnerSection(localNeighPos));
			}
			// The face is visible only if the block in the direction of the face is transparent
			// and different from the current block
			if (!ResManager::blockDatas().get(neighBlock.id).isOpaque() && neighBlock.id != block.id) {
				currBlock = block;
				// Ambient occlusion and smooth lighting share the same corner neighborhood, so they
				// are computed together in one pass.
				std::tie(currAO, currLight) = computeFaceLighting(neighbors, dir, localPos);
			}
		}
		// End the current face when the next block differs. A merged face has one AO and one light
		// value per corner, so a change in either ends it too.
		if (currBlock.id != lastBlock.id || currAO != lastAO || currLight != lastLight) {
			addFace(defaultVertices, waterVertices, lavaVertices, dir, localPos, firstBlockIndex, lastBlock, indexOfLastAxis, lastAO, lastLight);
			firstBlockIndex = localPos[indexOfLastAxis];
			lastBlock = currBlock;
			lastAO = currAO;
			lastLight = currLight;
		}
	}
	// Add the last face if at the end of last axis
	addFace(defaultVertices, waterVertices, lavaVertices, dir, localPos, firstBlockIndex, lastBlock, indexOfLastAxis, lastAO, lastLight);
}

void Section::addFace(std::vector<DefaultMesh::Vertex>& defaultVertices, std::vector<WaterMesh::Vertex>& waterVertices,
	std::vector<LavaMesh::Vertex>& lavaVertices,
	Dir3D::Dir dir, ivec3 localPos, int firstBlockIndex, Block block, int indexOfLastAxis,
	const std::array<GLfloat, 4>& ao, const std::array<vec2, 4>& light) const {

	BlockData::Category category = ResManager::blockDatas().get(block.id).getCategory();
	if (category != BlockData::AIR) {
		ivec3 negativeLastAxis{ 0, 0, 0 };
		negativeLastAxis[indexOfLastAxis] = -1;
		int length = localPos[indexOfLastAxis] - firstBlockIndex;
		ivec3 firstBlockGlobalPos{ Converter::sectionToGlobal(m_position) + localPos + negativeLastAxis * length };

		if (category == BlockData::DEFAULT || category == BlockData::SEMI_TRANSPARENT) {
			addDefaultFace(defaultVertices, dir, block, indexOfLastAxis, length, firstBlockGlobalPos, ao, light);
		}
		else if (category == BlockData::WATER) {
			addWaterFace(waterVertices, dir, indexOfLastAxis, length, firstBlockGlobalPos);
		}
		else if (category == BlockData::LAVA) {
			addLavaFace(lavaVertices, dir, indexOfLastAxis, length, firstBlockGlobalPos);
		}
	}
}

void Section::addDefaultFace(std::vector<DefaultMesh::Vertex>& defaultVertices, Dir3D::Dir dir, Block block,
	int indexOfLastAxis, int length, ivec3 firstBlockGlobalPos, const std::array<GLfloat, 4>& ao, const std::array<vec2, 4>& light) const {

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
		defaultVertices.push_back({ currVtx + vec3(firstBlockGlobalPos), tex, Dir3D::to_ivec3(dir), texID, ao[vtx], light[vtx] });
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

void Section::addLavaFace(std::vector<LavaMesh::Vertex>& lavaVertices, Dir3D::Dir dir,
	int indexOfLastAxis, int length, ivec3 firstBlockGlobalPos) const {

	for (int vtx = 0; vtx < 4; ++vtx) {
		vec3 currVtx = CubeData::dirToFace[dir][vtx];
		currVtx[indexOfLastAxis] *= length;
		lavaVertices.push_back({ currVtx + vec3(firstBlockGlobalPos), Dir3D::to_ivec3(dir) });
	}
}

std::pair<std::array<GLfloat, 4>, std::array<vec2, 4>> Section::computeFaceLighting(
		const NeighborChunks& neighbors, Dir3D::Dir dir, ivec3 localPos) const {
	const ivec3 n = Dir3D::to_ivec3(dir);
	// The axis the face points along, and the two axes spanning the face plane.
	const int normalAxis = (n.x != 0) ? 0 : (n.y != 0 ? 1 : 2);
	const int u = (normalAxis + 1) % 3;
	const int v = (normalAxis + 2) % 3;
	const ivec3 base = localPos + n; // One block out, into the open layer the face looks into.

	// Brightness per occlusion level: 0 = corner fully tucked between two blocks, 3 = open.
	static const std::array<GLfloat, 4> levelBrightness{ 0.5f, 0.7f, 0.85f, 1.f };

	auto sample = [&](ivec3 pos) -> std::pair<vec2, bool> {
		const Block b = getNeigboringBlock(neighbors, pos);
		const bool opaque = ResManager::blockDatas().get(b.id).isOpaque();
		return { vec2{ b.light.sky() / 15.f, b.light.block() / 15.f }, opaque };
	};

	// The cell in front of the face is always open, so it lights every corner.
	const vec2 baseLight = sample(base).first;

	std::array<GLfloat, 4> ao;
	std::array<vec2, 4> light;
	for (int vtx = 0; vtx < 4; ++vtx) {
		const vec3 corner = CubeData::dirToFace[dir][vtx];
		ivec3 du{ 0, 0, 0 }; du[u] = (corner[u] == 1) ? 1 : -1;
		ivec3 dv{ 0, 0, 0 }; dv[v] = (corner[v] == 1) ? 1 : -1;

		const auto [side1Light, side1Opaque] = sample(base + du);
		const auto [side2Light, side2Opaque] = sample(base + dv);
		const auto [cornLight, cornOpaque] = sample(base + du + dv);

		// Ambient occlusion: two side blocks fully close the corner, hiding whatever is diagonally
		// behind it; otherwise darken by how many of the three surrounding cells are solid.
		const int level = (side1Opaque && side2Opaque)
				? 0 : (3 - (side1Opaque + side2Opaque + cornOpaque));
		ao[vtx] = levelBrightness[level];

		// Smooth lighting: average the open cells around the corner. The diagonal counts only if a
		// side is open, since light can't slip through a solid corner.
		vec2 sum = baseLight;
		int count = 1;
		if (!side1Opaque) { sum += side1Light; ++count; }
		if (!side2Opaque) { sum += side2Light; ++count; }
		if (!(side1Opaque && side2Opaque) && !cornOpaque) { sum += cornLight; ++count; }
		light[vtx] = sum / float(count);
	}
	return { ao, light };
}

Block Section::getNeigboringBlock(const NeighborChunks& neighbors, ivec3 localPos) const {
	if (isInSection(localPos))
		return getBlock(localPos);

	auto axisCrossing = [](int coord) {
		return coord < 0 ? -1 : (coord >= Const::SECTION_SIDE ? 1 : 0);
	};

	// Resolve the horizontal crossing through the 3x3 grid; an unsnapshotted neighbor reads as air.
	const Chunk* chunk = neighbors.at(axisCrossing(localPos.x), axisCrossing(localPos.z));
	if (chunk == nullptr)
		return { BlockID::AIR };
	localPos.x = (localPos.x + Const::SECTION_SIDE) % Const::SECTION_SIDE;
	localPos.z = (localPos.z + Const::SECTION_SIDE) % Const::SECTION_SIDE;

	// Vertical crossing into another stacked section of the resolved chunk. A position outside the
	// populated range (or in a sparse gap) resolves to air inside getBlock().
	const int globalY = m_position.y * Const::SECTION_HEIGHT + localPos.y;
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
	// CPU only: build the vertex data. Runs on a worker thread, so it must not touch OpenGL. The
	// heavy face search runs lock-free; only the handoff into the shared "next" buffers is guarded.
	auto [defaultVertices, waterVertices, lavaVertices] = findVisibleFaces(neighbors);
	std::lock_guard<std::mutex> lock(m_meshMutex);
	m_nextDefaultVertices = std::move(defaultVertices);
	m_nextWaterVertices = std::move(waterVertices);
	m_nextLavaVertices = std::move(lavaVertices);
	m_meshReady = true;
}

void Section::uploadMesh() {
	// All OpenGL for the mesh happens here, on the main thread
	std::lock_guard<std::mutex> lock(m_meshMutex);
	if (!m_meshReady)
		return;
	nextDefaultMesh.loadBuffers(m_nextDefaultVertices, getIndices((int)m_nextDefaultVertices.size() / 4));
	nextWaterMesh.loadBuffers(m_nextWaterVertices, getIndices((int)m_nextWaterVertices.size() / 4));
	nextLavaMesh.loadBuffers(m_nextLavaVertices, getIndices((int)m_nextLavaVertices.size() / 4));
	nextDefaultMesh.loadVAOs();
	nextWaterMesh.loadVAOs();
	nextLavaMesh.loadVAOs();

	// Assigning frees the previous mesh's GPU resources: clear() runs on the old value first.
	activeDefaultMesh = std::move(nextDefaultMesh);
	activeWaterMesh = std::move(nextWaterMesh);
	activeLavaMesh = std::move(nextLavaMesh);

	// Free memory
	m_nextDefaultVertices = {};
	m_nextWaterVertices = {};
	m_nextLavaVertices = {};
	m_meshReady = false;

	Debug::glCheckError();
}

void Section::releaseMesh() {
	activeDefaultMesh.clear();
	activeWaterMesh.clear();
	activeLavaMesh.clear();
}

void Section::render(const Renderer& renderer) const {
	renderer.render(*this);
}

const DefaultMesh& Section::getDefaultMesh() const {
	return activeDefaultMesh;
}

const WaterMesh& Section::getWaterMesh() const {
	return activeWaterMesh;
}

const LavaMesh& Section::getLavaMesh() const {
	return activeLavaMesh;
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

Block& Section::getBlock(ivec3 pos) {
	return m_blocks->at(pos);
}

bool Section::isInSection(ivec3 globalPos) const {
	return 0 <= globalPos.x && globalPos.x < Const::SECTION_SIDE && 
			0 <= globalPos.y && globalPos.y < Const::SECTION_HEIGHT && 
			0 <= globalPos.z && globalPos.z < Const::SECTION_SIDE;
}