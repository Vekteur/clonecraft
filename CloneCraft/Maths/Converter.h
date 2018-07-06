#pragma once

#include <GlmCommon.h>

class Converter
{
public:
	Converter() = delete;

	static ivec3 chunkToGlobal(ivec2 pos);
	static ivec2 globalToChunk(ivec3 pos);
	static ivec2 globalToInnerChunk(ivec3 pos);
	static ivec3 sectionToGlobal(ivec3 pos);
	static ivec3 globalToSection(ivec3 pos);
	static ivec3 globalToInnerSection(ivec3 pos);
};

int floorDiv(int base, int divider);
int posMod(int base, int modulo);

template<typename T>
const T& clamp(const T& x, const T& upper, const T& lower) {
	return min(upper, max(x, lower));
}