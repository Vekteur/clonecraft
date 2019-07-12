#pragma once

#include "Util/Array3D.h"
#include "Block/Block.h"
#include "Generator/Noise/OctavePerlin.h"

#include "Maths/GlmCommon.h"

#include <optional>

class Chunk;

class Structure {
public:
	Structure() { }
	virtual ~Structure() = default;
	virtual bool isValidPos(ivec2 pos) const = 0;
	virtual Block getBlock(ivec3 pos) const = 0;
	virtual ivec3 size() const = 0;
	ivec2 getCenterPos(ivec2 globalPos) const;
};

