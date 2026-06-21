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
#include "Generator/Structure/Palm.h"
#include "Generator/Structure/DeadTree.h"
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
	addStructure(std::make_unique<Palm>(), StructureID::PALM);
	addStructure(std::make_unique<DeadTree>(), StructureID::DEAD_TREE);
	addStructure(std::make_unique<Cactus>(), StructureID::CACTUS);
}

BiomeID BiomeMap::getBiomeID(ivec2 pos) const {
	return getColumn(pos).biome;
}

const Biome& BiomeMap::getBiome(ivec2 pos) const {
	return getBiome(getBiomeID(pos));
}

const Biome& BiomeMap::getBiome(BiomeID biomeID) const {
	return *m_biomes[static_cast<int>(biomeID)];
}

int BiomeMap::getHeight(ivec2 pos) const {
	return getColumn(pos).height;
}

ColumnInfo BiomeMap::getColumn(ivec2 pos) const {
	double temperature = getTemperature(pos), altitude = getAltitude(pos);

	// One pass over biomes: pick the dominant one and gather weights to blend with.
	std::array<double, static_cast<int>(BiomeID::SIZE)> weights;
	double sumValues = 0., maxValue = 0.;
	BiomeID dominant = BiomeID::SIZE;
	for (int i = 0; i < static_cast<int>(BiomeID::SIZE); ++i) {
		double value = m_biomes[i]->biomeValue(temperature, altitude);
		weights[i] = value;
		sumValues += value;
		if (value > maxValue) {
			maxValue = value;
			dominant = static_cast<BiomeID>(i);
		}
	}
	if (dominant == BiomeID::SIZE)
		throw "Biome not found";

	// Blend height and ruggedness by each biome's share of the total weight.
	double height = 0., ruggedness = 0.;
	for (int i = 0; i < static_cast<int>(BiomeID::SIZE); ++i) {
		if (weights[i] == 0.)
			continue;
		double share = weights[i] / sumValues;
		height += share * m_biomes[i]->getHeight(pos);
		ruggedness += share * m_biomes[i]->ruggedness();
	}
	return { dominant, static_cast<int>(height), ruggedness };
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
	return m_temperatureNoise.getNoise(static_cast<dvec2>(pos));
}

double BiomeMap::getAltitude(ivec2 pos) const {
	return m_altitudeNoise.getNoise(static_cast<dvec2>(pos + 173));
}
