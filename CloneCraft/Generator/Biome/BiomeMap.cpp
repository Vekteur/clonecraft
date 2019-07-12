#include "BiomeMap.h"

#include "Generator/Noise/PerlinNoise.h"

#include "Islands.h"
#include "Plain.h"

#include "Generator/Structure/Tree.h"

BiomeMap::BiomeMap() {
	addBiome(std::make_unique<Plain>(), BiomeID::PLAIN);
	addBiome(std::make_unique<Islands>(), BiomeID::ISLANDS);

	addStructure(std::make_unique<Tree>(), StructureID::TREE);
}

BiomeID BiomeMap::getBiomeID(ivec2 pos) const {
	for (int biomeID = 0; biomeID < static_cast<int>(BiomeID::SIZE); ++biomeID) {
		if (m_biomes[biomeID]->isInBiome(getTemperature(pos), getHumidity(pos))) {
			return static_cast<BiomeID>(biomeID);
		}
	}
	throw "Biome not found";
}

const Biome& BiomeMap::getBiome(ivec2 pos) const {
	return getBiome(getBiomeID(pos));
}

const Biome& BiomeMap::getBiome(BiomeID biomeID) const {
	return *m_biomes[static_cast<int>(biomeID)];
}

int BiomeMap::getHeight(ivec2 pos) const {
	return getBiome(pos).getHeight(pos);
}

const Structure& BiomeMap::getStructure(StructureID structureID) const {
	return *m_structures[static_cast<int>(structureID)];
}

void BiomeMap::addBiome(std::unique_ptr<Biome> biome, BiomeID biomeID) {
	m_biomes[static_cast<int>(biomeID)] = std::move(biome);
}

void BiomeMap::addStructure(std::unique_ptr<Structure> structure, StructureID structureID) {
	m_structures[static_cast<int>(structureID)] = std::move(structure);
}

double BiomeMap::getTemperature(ivec2 pos) const {
	return PerlinNoise::getNoise(static_cast<dvec2>(pos), 1. / 512.);
}

double BiomeMap::getHumidity(ivec2 pos) const {
	// TODO : Make a different perlin noise
	return PerlinNoise::getNoise(static_cast<dvec2>(pos + 173), 1. / 683.);
}
