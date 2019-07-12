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
#include "Util/Logger.h"

BiomeMap::BiomeMap() {
	addBiome(std::make_unique<ExtremeMountains>(), BiomeID::EXTREME_MOUNTAINS);
	addBiome(std::make_unique<Mountains>(), BiomeID::MOUNTAINS);
	addBiome(std::make_unique<Desert>(), BiomeID::DESERT);
	addBiome(std::make_unique<Snow>(), BiomeID::SNOW);
	addBiome(std::make_unique<Forest>(), BiomeID::FOREST);
	addBiome(std::make_unique<Plain>(), BiomeID::PLAIN);
	addBiome(std::make_unique<Ocean>(), BiomeID::OCEAN);
	addBiome(std::make_unique<Islands>(), BiomeID::ISLANDS);
	addBiome(std::make_unique<Swamp>(), BiomeID::SWAMP);

	addStructure(std::make_unique<Tree>(), StructureID::TREE);
}

BiomeID BiomeMap::getBiomeID(ivec2 pos) const {
	double maxValue = 0.;
	BiomeID chosenbiomeID = BiomeID::SIZE;
	for (int biomeID = 0; biomeID < static_cast<int>(BiomeID::SIZE); ++biomeID) {
		double value = m_biomes[biomeID]->biomeValue(getTemperature(pos), getHumidity(pos));
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
		double value = m_biomes[biomeID]->biomeValue(getTemperature(pos), getHumidity(pos));
		sumValues += value;
		if (value != 0.)
			values.push_back({ static_cast<BiomeID>(biomeID), value });
	}

	//LOG(Level::INFO) << values.size() << " " << sumValues << std::endl;

	double height = 0.;
	for (auto& p : values) {
		BiomeID biomeID; double value;
		std::tie(biomeID, value) = p;
		height += (value / sumValues) * getBiome(biomeID).getHeight(pos);
	}
	return static_cast<int>(height);
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
