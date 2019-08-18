#include "ChunkGenerationInfo.h"

int ChunkGenerationInfo::height(ivec2 pos) const {
	return m_height[pos.x][pos.y];
}

int& ChunkGenerationInfo::height(ivec2 pos) {
	return m_height[pos.x][pos.y];
}

BiomeID ChunkGenerationInfo::biome(ivec2 pos) const {
	return m_biome[pos.x][pos.y];
}

BiomeID& ChunkGenerationInfo::biome(ivec2 pos) {
	return m_biome[pos.x][pos.y];
}
