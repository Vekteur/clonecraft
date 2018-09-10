#pragma once

#include <glad\glad.h>
#include <GlmCommon.h>

#include <array>
#include <vector>
#include <optional>

#include "Array3D.h"
#include "Dir3D.h"
#include "Block.h"
#include "Mesh.h"
#include "DefaultRenderer.h"
#include "WaterRenderer.h"
#include "WorldConstants.h"

class ChunkMap;
class Chunk;

class Section {
public:
	Section(const ChunkMap* chunkMap = nullptr, const Chunk* chunk = nullptr, ivec3 position = ivec3{ 0, 0, 0 });
	Section(Section&& other) = default;
	Section& operator=(Section&& other) = default;
	Section(const Section& other) = delete;
	Section& operator=(const Section& other) = delete;

	void loadFaces();
	void loadVAOs();
	void unloadVAOs();
	void render(const DefaultRenderer& shader) const;
	void render(const WaterRenderer& waterRenderer) const;

	ivec3 getPosition() const;
	void setBlock(ivec3 pos, Block block);
	Block getBlock(ivec3 pos) const;

private:
	using BlockArray = Array3D<Block, Const::SECTION_SIDE, Const::SECTION_HEIGHT, Const::SECTION_SIDE>;
	const ChunkMap* p_chunkMap{ nullptr };
	const Chunk* p_chunk{ nullptr };
	ivec3 m_position;

	std::unique_ptr<BlockArray> m_blocks;
	DefaultMesh activeDefaultMesh;
	DefaultMesh nextDefaultMesh;
	WaterMesh activeWaterMesh;
	WaterMesh nextWaterMesh;
	bool empty = true;

	bool isInSection(ivec3 block);
	std::tuple<std::vector<DefaultMesh::Vertex>, std::vector<WaterMesh::Vertex>>  findFaces();
};