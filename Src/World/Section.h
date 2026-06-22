#pragma once

#include <glad/glad.h>
#include <Maths/GlmCommon.h>

#include <array>
#include <vector>
#include <optional>
#include <mutex>

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
using NeighborChunks = std::array<const Chunk*, Dir2D::SIZE>;

class Section {
public:
	Section(const Chunk* chunk = nullptr, ivec3 position = ivec3{ 0, 0, 0 });
	// Non-copyable and non-movable: a Section lives in place inside Chunk's section map (inserted with
	// try_emplace, never relocated) and holds a mutex, so neither copy nor move would be valid.
	Section(const Section& other) = delete;
	Section& operator=(const Section& other) = delete;

	void loadMesh(const NeighborChunks& neighbors);
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
	// Built on a worker thread by loadMesh(); uploaded to the GPU on the main thread by uploadMesh().
	std::vector<DefaultMesh::Vertex> m_nextDefaultVertices;
	std::vector<WaterMesh::Vertex> m_nextWaterVertices;
	// Serializes producers/consumer of the "next" vertex buffers above. A single block edit (main
	// thread) and a bulk edit (worker) can remesh the same section at once, and the main thread
	// uploads it; this guards those vectors from concurrent build/build and build/upload.
	mutable std::mutex m_meshMutex;

	bool isInSection(ivec3 globalPos) const;
	const Section* findNeighboringSection(Dir3D::Dir dir, const NeighborChunks& neighbors) const;
	std::tuple<std::vector<DefaultMesh::Vertex>, std::vector<WaterMesh::Vertex>> findVisibleFaces(const NeighborChunks& neighbors) const;
	void addVisibleFacesOnLastAxis(
		std::vector<DefaultMesh::Vertex>& defaultVertices, std::vector<WaterMesh::Vertex>& waterVertices,
		Dir3D::Dir dir, ivec3 localPos, int indexOfLastAxis, int sizeOfLastAxis,
		const Section* neighboringSection, const NeighborChunks& neighbors
	) const;
	void addFace(std::vector<DefaultMesh::Vertex>& defaultVertices, std::vector<WaterMesh::Vertex>& waterVertices,
		Dir3D::Dir dir, ivec3 localPos, int length, Block block, int indexOfLastAxis,
		const std::array<GLfloat, 4>& ao) const;
	void addDefaultFace(std::vector<DefaultMesh::Vertex>& defaultVertices, Dir3D::Dir dir, Block block,
		int indexOfLastAxis, int length, ivec3 firstBlockGlobalPos, const std::array<GLfloat, 4>& ao) const;
	// Ambient occlusion: brightness (0..1) for each of a face's 4 corners, darkened by the solid
	// blocks diagonally around the corner. Computed per unit-block face during meshing.
	std::array<GLfloat, 4> computeFaceAO(const NeighborChunks& neighbors, Dir3D::Dir dir, ivec3 localPos) const;
	// Block at a position in this section's local coordinates, resolving across neighboring sections
	Block getBlockForAO(const NeighborChunks& neighbors, ivec3 localPos) const;
	void addWaterFace(std::vector<WaterMesh::Vertex>& waterVertices, Dir3D::Dir dir,
		int indexOfLastAxis, int length, ivec3 firstBlockGlobalPos) const;
};