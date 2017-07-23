#include "ConvertPos.h"
#include "Chunk.h"
#include "Section.h"

ivec3 ConvertPos::chunkToGlobal(ivec2 pos)
{
	return ivec3{ pos.x * Chunk::SIDE, 0, pos.y * Chunk::SIDE };
}

ivec2 ConvertPos::globalToChunk(ivec3 pos)
{
	return ivec2{ pos.x / Chunk::SIDE, pos.z / Chunk::SIDE };
}

ivec3 ConvertPos::sectionToGlobal(ivec3 pos)
{
	return ivec3{ pos.x * Chunk::SIDE, pos.y * Section::HEIGHT, pos.z * Chunk::SIDE };
}

ivec3 ConvertPos::globalToSection(ivec3 pos)
{
	return ivec3{ pos.x / Chunk::SIDE, pos.y / Section::HEIGHT, pos.z / Chunk::SIDE };
}