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

	// Large-scale surface height of a column.
	virtual int getHeight(ivec2 pos) const = 0;

	// Block to place in solid ground. 'depth' is the number of solid blocks above this one in the
	// same column (0 means the surface), which lets biomes layer grass/dirt/stone, even on overhangs.
	virtual Block getBlock(ivec3 pos, int depth) const = 0;

	// How much the 3D noise distorts the surface, in blocks. 0 keeps the biome flat (a plain),
	// higher values carve cliffs and overhangs (mountains).
	virtual double ruggedness() const { return 0.; }

	// Block used for the topmost liquid layer below sea level (water by default, ice for snow).
	virtual BlockID surfaceFluid() const { return BlockID::WATER; }

	virtual std::vector<StructureInfo> getStructures() const = 0;
	virtual double biomeValue(double temperature, double altitude) const = 0;

protected:
	static double threshold, transition;

	// Common ground layering: a beach of sand around the waterline, otherwise 'surface' on top,
	// a few blocks of 'subsurface' under it, then stone.
	static Block layeredGround(ivec3 pos, int depth, BlockID surface, BlockID subsurface);

	static double low(double value);
	static double medium(double value);
	static double high(double value);
	static double step(double value, double threshold);
	static double smooth(double x);
};