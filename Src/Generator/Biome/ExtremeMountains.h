#pragma once

#include "Generator/Biome/Biome.h"
#include "Generator/Noise/OctavePerlin.h"

class ExtremeMountains : public Biome {
	virtual int getHeight(ivec2 pos) const override;
	virtual Block getBlock(ivec3 pos, int depth) const override;
	virtual double ruggedness() const override { return 50.; }
	virtual std::vector<StructureInfo> getStructures() const override;
	virtual double biomeValue(double temperature, double altitude) const override;

private:
	OctavePerlin perlin{ 5, 0.45, 1. / 280. };
	OctavePerlin warp{ 2, 0.5, 1. / 220. };
	OctavePerlin snowPerlin{ 2, 0.5, 1. / 100. };
};