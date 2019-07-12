#pragma once

#include "Maths/GlmCommon.h"
#include "Block/Block.h"
#include "Generator/Biome/BiomeID.h"
#include "Generator/Structure/StructureID.h"

#include <vector>

struct StructureInfo {
	StructureID id;
	float freq;
};

class Biome {
public:
	virtual ~Biome() = default;
	virtual int getHeight(ivec2 pos) const = 0;
	virtual Block getBlock(int y, int height) const = 0;
	virtual std::vector<StructureInfo> getStructures() const = 0;
	virtual bool isInBiome(double temperature, double humidity) const = 0;

protected:
	double threshold = 0.14;

	bool low(double value) const;
	bool medium(double value) const;
	bool high(double value) const;
};