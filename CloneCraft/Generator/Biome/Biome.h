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
	virtual double biomeValue(double temperature, double humidity) const = 0;

protected:
	static double threshold, transition;

	static double low(double value);
	static double medium(double value);
	static double high(double value);
	static double step(double value, double threshold);
	static double smooth(double x);
};