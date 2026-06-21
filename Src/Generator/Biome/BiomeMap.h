#pragma once

#include "Generator/Biome/Biome.h"
#include "Generator/Biome/BiomeID.h"
#include "Generator/Structure/Structure.h"
#include "Generator/Structure/StructureID.h"
#include "Generator/Noise/OctavePerlin.h"
#include "Maths/GlmCommon.h"

#include <memory>
#include <array>

// Everything the generator needs about a terrain column, computed in one pass.
struct ColumnInfo {
	BiomeID biome;
	int height;
	double ruggedness;
};

class BiomeMap {
public:
	BiomeMap();
	BiomeID getBiomeID(ivec2 pos) const;
	const Biome& getBiome(ivec2 pos) const;
	const Biome& getBiome(BiomeID biomeID) const;
	int getHeight(ivec2 pos) const;
	ColumnInfo getColumn(ivec2 pos) const;
	std::string getBiomeName(ivec2 pos) const;
	const Structure& getStructure(StructureID structureID) const;

private:
	std::array<std::unique_ptr<Biome>, static_cast<int>(BiomeID::SIZE)> m_biomes;
	std::array<std::string, static_cast<int>(BiomeID::SIZE)> m_biome_names;
	std::array<std::unique_ptr<Structure>, static_cast<int>(StructureID::SIZE)> m_structures;

	void addBiome(std::unique_ptr<Biome> biome, BiomeID biomeID, std::string name);
	void addStructure(std::unique_ptr<Structure> structure, StructureID structureID);
	double getTemperature(ivec2 pos) const;
	double getAltitude(ivec2 pos) const;

	OctavePerlin m_temperatureNoise{ 4, 0.5, 1. / 1024. };
	OctavePerlin m_altitudeNoise{ 4, 0.7, 1. / 1353. };
};

