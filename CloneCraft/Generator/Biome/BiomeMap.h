#pragma once

#include "Generator/Biome/Biome.h"
#include "Generator/Biome/BiomeID.h"
#include "Generator/Structure/Structure.h"
#include "Generator/Structure/StructureID.h"
#include "Maths/GlmCommon.h"

#include <memory>
#include <array>

class BiomeMap {
public:
	BiomeMap();
	BiomeID getBiomeID(ivec2 pos) const;
	const Biome& getBiome(ivec2 pos) const;
	const Biome& getBiome(BiomeID biomeID) const;
	int getHeight(ivec2 pos) const;
	const Structure& getStructure(StructureID structureID) const;

private:
	std::array<std::unique_ptr<Biome>, static_cast<int>(BiomeID::SIZE)> m_biomes;
	std::array<std::unique_ptr<Structure>, static_cast<int>(StructureID::SIZE)> m_structures;

	void addBiome(std::unique_ptr<Biome> biome, BiomeID biomeID);
	void addStructure(std::unique_ptr<Structure> structure, StructureID structureID);
	double getTemperature(ivec2 pos) const;
	double getHumidity(ivec2 pos) const;
};

