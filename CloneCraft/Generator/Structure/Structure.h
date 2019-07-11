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
	virtual std::optional<ivec3> getLocalPos(ivec2 zonePos, vec2 freq, const Chunk& chunk) const = 0;
	virtual Block getBlock(ivec3 pos) const = 0;
	virtual ivec3 size() const = 0;
};

