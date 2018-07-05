#include "Converter.h"
#include "Chunk.h"
#include "Section.h"

ivec3 Converter::chunkToGlobal(ivec2 pos) {
	return ivec3{ pos.x * Const::CHUNK_SIDE, 0, pos.y * Const::CHUNK_SIDE };
}

ivec2 Converter::globalToChunk(ivec3 pos) {
	return ivec2{ floorDiv(pos.x, Const::CHUNK_SIDE), floorDiv(pos.z, Const::CHUNK_SIDE) };
}

ivec3 Converter::sectionToGlobal(ivec3 pos) {
	return ivec3{ pos.x * Const::SECTION_SIDE, pos.y * Const::SECTION_HEIGHT, pos.z * Const::SECTION_SIDE };
}

ivec3 Converter::globalToSection(ivec3 pos) {
	return ivec3{ floorDiv(pos.x, Const::SECTION_SIDE), floorDiv(pos.y, Const::SECTION_HEIGHT), floorDiv(pos.z, Const::SECTION_SIDE) };
}

int Converter::floorDiv(int base, int divider) {
	return base >= 0 ? base / divider : (base - divider + 1) / divider;
}

int posMod(int base, int modulo) {
	return ((base % modulo) + modulo) % modulo;
}