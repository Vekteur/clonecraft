#pragma once

#include <glad\glad.h>
#include <GlmCommon.h>

#include <array>
#include <vector>

#include "Array3D.h"
#include "Texture.h"
#include "Dir3D.h"
#include "Block.h"
#include "Mesh.h"
#include "DefaultRenderer.h"

class ChunkMap;
class Chunk;

class Section
{
public:
	Section(ChunkMap* const chunkMap, Chunk* const chunk, ivec3 position = ivec3{ 0, 0, 0 });

	void loadBlocks();
	void loadFaces();
	void loadVAOs();
	void unloadVAOs();
	void render(DefaultRenderer &shader) const;

	void setBlock(ivec3 pos, Block block);
	Block getBlock(ivec3 pos) const;

private:
	ChunkMap* const p_chunkMap{ nullptr };
	Chunk* const p_chunk{ nullptr };
	const ivec3 m_position;

	Array3D<Block> m_blocks;
	DefaultMesh defaultMesh;
	bool empty = true;

	bool isInSection(ivec3 block);
	std::vector<DefaultMesh::Vertex> findFaces();
};