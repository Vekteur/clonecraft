#include "Converter.h"
#include "World/Chunk.h"
#include "World/Section.h"

ivec3 Converter::chunkToGlobal(ivec2 pos) {
	return { pos.x * Const::SECTION_SIDE, 0, pos.y * Const::SECTION_SIDE };
}

ivec2 Converter::globalToChunk(ivec3 pos) {
	return { floorDiv(pos.x, Const::SECTION_SIDE), floorDiv(pos.z, Const::SECTION_SIDE) };
}

ivec2 Converter::globalToInnerChunk(ivec3 pos) {
	return { posMod(pos.x, Const::SECTION_SIDE), posMod(pos.z, Const::SECTION_SIDE) };
}

vec2 Converter::globalToInnerChunk(vec3 pos) {
	return { posMod(pos.x, float(Const::SECTION_SIDE)), posMod(pos.z, float(Const::SECTION_SIDE)) };
}

ivec3 Converter::sectionToGlobal(ivec3 pos) {
	return { pos.x * Const::SECTION_SIDE, pos.y * Const::SECTION_HEIGHT, pos.z * Const::SECTION_SIDE };
}

ivec3 Converter::globalToSection(ivec3 pos) {
	return { floorDiv(pos.x, Const::SECTION_SIDE), floorDiv(pos.y, Const::SECTION_HEIGHT), floorDiv(pos.z, Const::SECTION_SIDE) };
}

ivec3 Converter::globalToInnerSection(ivec3 pos) {
	return { posMod(pos.x, Const::SECTION_SIDE), posMod(pos.y, Const::SECTION_HEIGHT), posMod(pos.z, Const::SECTION_SIDE) };
}

vec3 Converter::globalToInnerSection(vec3 pos) {
	return { posMod(pos.x, float(Const::SECTION_SIDE)), posMod(pos.y, float(Const::SECTION_HEIGHT)), posMod(pos.z, float(Const::SECTION_SIDE)) };
}

ivec3 Converter::globalPosToBlock(vec3 pos) {
	return floor(pos);
}

ivec2 Converter::to2D(ivec3 pos) {
	return { pos.x, pos.z };
}

ivec3 Converter::to3D(ivec2 pos) {
	return { pos.x, 0, pos.y };
}

int floorDiv(int base, int divider) {
	return (base >= 0 ? base : (base - divider + 1)) / divider;
}

int floorDiv(float base, float divider) {
	return static_cast<int>(floor(base / divider));
}

int posMod(int base, int modulo) {
	return ((base % modulo) + modulo) % modulo;
}

float posMod(float base, float modulo) {
	return fmod(fmod(base, modulo) + modulo, modulo);
}
