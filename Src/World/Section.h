#pragma once

#include <glad/glad.h>
#include <Maths/GlmCommon.h>

#include <array>
#include <vector>
#include <optional>

#include "Util/Array3D.h"
#include "Maths/Dir2D.h"
#include "Maths/Dir3D.h"
#include "Block/Block.h"
#include "Mesh.h"
#include "Renderer/DefaultRenderer.h"
#include "Renderer/WaterRenderer.h"
#include "WorldConstants.h"

class Chunk;

// Chunks around a section's own chunk, snapshotted so sections never read the chunk map.
using NeighbourChunks = std::array<const Chunk*, Dir2D::SIZE>;

class Section {
public:
	Section(const Chunk* chunk = nullptr, ivec3 position = ivec3{ 0, 0, 0 });
	Section(Section&& other) = default;
	Section& operator=(Section&& other) = default;
	Section(const Section& other) = delete;
	Section& operator=(const Section& other) = delete;

	void loadMesh(const NeighbourChunks& neighbours);
	void uploadMesh();
	void releaseMesh();
	void render(const DefaultRenderer& shader) const;
	void render(const WaterRenderer& waterRenderer) const;

	ivec3 getPosition() const;
	void setBlock(ivec3 pos, Block block);
	Block getBlock(ivec3 pos) const;

private:
	using BlockArray = Array3D<Block, Const::SECTION_SIDE, Const::SECTION_HEIGHT, Const::SECTION_SIDE>;
	const Chunk* p_chunk{ nullptr };
	ivec3 m_position;

	std::unique_ptr<BlockArray> m_blocks;
	DefaultMesh activeDefaultMesh;
	DefaultMesh nextDefaultMesh;
	WaterMesh activeWaterMesh;
	WaterMesh nextWaterMesh;
	// Built on a worker thread by loadChunkMesh(); uploaded to the GPU on the main thread by uploadMesh().
	std::vector<DefaultMesh::Vertex> m_nextDefaultVertices;
	std::vector<WaterMesh::Vertex> m_nextWaterVertices;

	bool isInSection(ivec3 globalPos) const;
	const Section* findNeighboringSection(Dir3D::Dir dir, const NeighbourChunks& neighbours) const;
	std::tuple<std::vector<DefaultMesh::Vertex>, std::vector<WaterMesh::Vertex>> findVisibleFaces(const NeighbourChunks& neighbours) const;
	void addVisibleFacesOnLastAxe(
		std::vector<DefaultMesh::Vertex>& defaultVertices, std::vector<WaterMesh::Vertex>& waterVertices,
		Dir3D::Dir dir, ivec3 localPos, int indexOfLastAxe, int sizeOfLastAxe,
		const Section* neighboringSection
	) const;
	void addFace(std::vector<DefaultMesh::Vertex>& defaultVertices, std::vector<WaterMesh::Vertex>& waterVertices,
		Dir3D::Dir dir, ivec3 localPos, int length, Block block, int indexOfLastAxe) const;
	void addDefaultFace(std::vector<DefaultMesh::Vertex>& defaultVertices, Dir3D::Dir dir, Block block,
		int indexOfLastAxe, int length, ivec3 firstBlockGlobalPos) const;
	void addWaterFace(std::vector<WaterMesh::Vertex>& waterVertices, Dir3D::Dir dir,
		int indexOfLastAxe, int length, ivec3 firstBlockGlobalPos) const;
};