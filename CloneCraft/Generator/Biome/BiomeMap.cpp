#include "BiomeMap.h"

#include "Generator/Noise/PerlinNoise.h"

#include "ExtremeMountains.h"
#include "Mountains.h"
#include "Desert.h"
#include "Snow.h"
#include "Forest.h"
#include "Plain.h"
#include "Ocean.h"
#include "Islands.h"
#include "Swamp.h"

#include "Generator/Structure/Tree.h"
#include "Generator/Structure/Fir.h"
#include "Generator/Structure/Cactus.h"
#include "Util/Logger.h"

BiomeMap::BiomeMap() {
	addBiome(std::make_unique<ExtremeMountains>(), BiomeID::EXTREME_MOUNTAINS, "Extreme mountains");
	addBiome(std::make_unique<Mountains>(), BiomeID::MOUNTAINS, "Mountains");
	addBiome(std::make_unique<Desert>(), BiomeID::DESERT, "Desert");
	addBiome(std::make_unique<Snow>(), BiomeID::SNOW, "Snow");
	addBiome(std::make_unique<Forest>(), BiomeID::FOREST, "Forest");
	addBiome(std::make_unique<Plain>(), BiomeID::PLAIN, "Plain");
	addBiome(std::make_unique<Ocean>(), BiomeID::OCEAN, "Ocean");
	addBiome(std::make_unique<Islands>(), BiomeID::ISLANDS, "Islands");
	addBiome(std::make_unique<Swamp>(), BiomeID::SWAMP, "Swamp");

	addStructure(std::make_unique<Tree>(), StructureID::TREE);
	addStructure(std::make_unique<Fir>(), StructureID::FIR);
	addStructure(std::make_unique<Cactus>(), StructureID::CACTUS);
}

BiomeID BiomeMap::getBiomeID(ivec2 pos) const {
	double maxValue = 0.;
	BiomeID chosenbiomeID = BiomeID::SIZE;
	for (int biomeID = 0; biomeID < static_cast<int>(BiomeID::SIZE); ++biomeID) {
		double value = m_biomes[biomeID]->biomeValue(getTemperature(pos), getAltitude(pos));
		if (value > maxValue) {
			maxValue = value;
			chosenbiomeID = static_cast<BiomeID>(biomeID);
		}
	}
	if (chosenbiomeID == BiomeID::SIZE)
		throw "Biome not found";
	return chosenbiomeID;
}

const Biome& BiomeMap::getBiome(ivec2 pos) const {
	return getBiome(getBiomeID(pos));
}

const Biome& BiomeMap::getBiome(BiomeID biomeID) const {
	return *m_biomes[static_cast<int>(biomeID)];
}

int BiomeMap::getHeight(ivec2 pos) const {
	double sumValues = 0.;
	std::vector<std::tuple<BiomeID, double>> values;
	for (int biomeID = 0; biomeID < static_cast<int>(BiomeID::SIZE); ++biomeID) {
		double value = m_biomes[biomeID]->biomeValue(getTemperature(pos), getAltitude(pos));
		sumValues += value;
		if (value != 0.)
			values.push_back({ static_cast<BiomeID>(biomeID), value });
	}

	double height = 0.;
	for (auto& p : values) {
		BiomeID biomeID; double value;
		std::tie(biomeID, value) = p;
		height += (value / sumValues) * getBiome(biomeID).getHeight(pos);
	}
	return static_cast<int>(height);
}

std::string BiomeMap::getBiomeName(ivec2 pos) const {
	return m_biome_names[static_cast<int>(getBiomeID(pos))];
}

const Structure& BiomeMap::getStructure(StructureID structureID) const {
	return *m_structures[static_cast<int>(structureID)];
}

void BiomeMap::addBiome(std::unique_ptr<Biome> biome, BiomeID biomeID, std::string name) {
	m_biomes[static_cast<int>(biomeID)] = std::move(biome);
	m_biome_names[static_cast<int>(biomeID)] = name;
}

void BiomeMap::addStructure(std::unique_ptr<Structure> structure, StructureID structureID) {
	m_structures[static_cast<int>(structureID)] = std::move(structure);
}

double BiomeMap::getTemperature(ivec2 pos) const {
	OctavePerlin perlin{ 4, 0.5, 1. / 1024. };
	return perlin.getNoise(static_cast<dvec2>(pos));
	//return PerlinNoise::getNoise(static_cast<dvec2>(pos), 1. / 512.);
}

double BiomeMap::getAltitude(ivec2 pos) const {
	// TODO : Make a different perlin noise
	OctavePerlin perlin{ 4, 0.7, 1. / 1353. };
	return perlin.getNoise(static_cast<dvec2>(pos + 173));
	//return PerlinNoise::getNoise(static_cast<dvec2>(pos + 173), 1. / 683.);
}
