#pragma once

#include <Maths/GlmCommon.h>

class Converter
{
public:
	Converter() = delete;

	static ivec3 chunkToGlobal(ivec2 pos);
	static ivec2 globalToChunk(ivec3 pos);
	static ivec2 globalToInnerChunk(ivec3 pos);
	static vec2 globalToInnerChunk(vec3 pos);
	static ivec3 sectionToGlobal(ivec3 pos);
	static ivec3 globalToSection(ivec3 pos);
	static ivec3 globalToInnerSection(ivec3 pos);
	static vec3 globalToInnerSection(vec3 pos);
	static ivec3 globalPosToBlock(vec3 pos);
	static ivec2 to2D(ivec3 pos);
	static ivec3 to3D(ivec2 pos);
};

int floorDiv(int base, int divider);
int floorDiv(float base, float divider);
int posMod(int base, int modulo);
float posMod(float base, float modulo);

template<typename T>
const T& clamp(const T& x, const T& lower, const T& upper) {
	return std::max(lower, std::min(x, upper));
}