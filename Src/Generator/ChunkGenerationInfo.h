#pragma once

#include "World/WorldConstants.h"
#include "Generator/Biome/BiomeID.h"
#include "Maths/GlmCommon.h"

#include <vector>
#include <array>

class ChunkGenerationInfo {
public:
	int height(ivec2 pos) const;
	int& height(ivec2 pos);
	BiomeID biome(ivec2 pos) const;
	BiomeID& biome(ivec2 pos);

private:
	template<typename T, int S>
	using array2D = std::array<std::array<T, S>, S>;

	array2D<int, Const::SECTION_SIDE> m_height;
	array2D<BiomeID, Const::SECTION_SIDE> m_biome;
};