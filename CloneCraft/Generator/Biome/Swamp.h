#pragma once

#include "Generator/Biome/Biome.h"
#include "Generator/Noise/OctavePerlin.h"

class Swamp : public Biome {
	virtual int getHeight(ivec2 pos) const override;
	virtual Block getBlock(int y, int height) const override;
	virtual std::vector<StructureInfo> getStructures() const override;
	virtual double biomeValue(double temperature, double humidity) const override;

private:
	OctavePerlin perlin{ 4, 0.5, 1. / 128. };
};