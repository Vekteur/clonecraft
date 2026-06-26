#pragma once

#include <Maths/GlmCommon.h>

#include <cmath>

class Converter
{
public:
	Converter() = delete;

	static ivec3 chunkToGlobal(ivec2 pos);
	static ivec2 globalToChunk(ivec3 pos);
	static ivec3 globalToInnerChunk(ivec3 pos);
	static vec3 globalToInnerChunk(vec3 pos);
	static ivec2 globalToInnerChunk2D(ivec3 pos);
	static vec2 globalToInnerChunk2D(vec3 pos);
	static ivec3 sectionToGlobal(ivec3 pos);
	static ivec3 globalToSection(ivec3 pos);
	static ivec3 globalToInnerSection(ivec3 pos);
	static vec3 globalToInnerSection(vec3 pos);
	static ivec3 globalPosToBlock(vec3 pos);
	static ivec2 to2D(ivec3 pos);
	static ivec3 to3D(ivec2 pos);
};

inline int floorDiv(int base, int divider) {
	return (base >= 0 ? base : (base - divider + 1)) / divider;
}

inline int floorDiv(float base, float divider) {
	return static_cast<int>(std::floor(base / divider));
}

inline int posMod(int base, int modulo) {
	return ((base % modulo) + modulo) % modulo;
}

inline float posMod(float base, float modulo) {
	return std::fmod(std::fmod(base, modulo) + modulo, modulo);
}

template<typename T>
const T& clamp(const T& x, const T& lower, const T& upper) {
	return std::max(lower, std::min(x, upper));
}