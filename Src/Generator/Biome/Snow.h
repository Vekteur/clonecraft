#pragma once

#include "Generator/Biome/Biome.h"
#include "Generator/Noise/OctavePerlin.h"

class Snow : public Biome {
	virtual int getHeight(ivec2 pos) const override;
	virtual Block getBlock(ivec3 pos, int depth) const override;
	virtual BlockID surfaceFluid() const override { return BlockID::ICE; }
	virtual std::vector<StructureInfo> getStructures() const override;
	virtual double biomeValue(double temperature, double altitude) const override;

private:
	OctavePerlin perlin{ 4, 0.5, 1. / 128. };
};