#include "Converter.h"
#include "Chunk.h"
#include "Section.h"

ivec3 Converter::chunkToGlobal(ivec2 pos)
{
	return ivec3{ pos.x * Chunk::SIDE, 0, pos.y * Chunk::SIDE };
}

ivec2 Converter::globalToChunk(ivec3 pos)
{
	return ivec2{ floorDiv(pos.x, Chunk::SIDE), floorDiv(pos.z, Chunk::SIDE) };
}

ivec3 Converter::sectionToGlobal(ivec3 pos)
{
	return ivec3{ pos.x * Chunk::SIDE, pos.y * Section::HEIGHT, pos.z * Chunk::SIDE };
}

ivec3 Converter::globalToSection(ivec3 pos)
{
	return ivec3{ floorDiv(pos.x, Chunk::SIDE), floorDiv(pos.y, Section::HEIGHT), floorDiv(pos.z, Chunk::SIDE) };
}

int Converter::positiveMod(int base, int modulo)
{
	return ((base % modulo) + modulo) % modulo;
}

int Converter::floorDiv(int base, int divider)
{
	return base >= 0 ? base / divider : (base - divider + 1) / divider;
}
