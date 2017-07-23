#pragma once

#include <GlmCommon.h>

class ConvertPos
{
public:
	ConvertPos() = delete;

	static ivec3 chunkToGlobal(ivec2 pos);
	static ivec2 globalToChunk(ivec3 pos);
	static ivec3 sectionToGlobal(ivec3 pos);
	static ivec3 globalToSection(ivec3 pos);
};