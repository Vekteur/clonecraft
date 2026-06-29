#include "Cherry.h"

#include "TreeShapes.h"
#include "Generator/WorldGenerator.h"
#include "Maths/Dir3D.h"

#include <algorithm>
#include <cmath>

namespace {
	constexpr int WIDTH = 17;
	constexpr int HEIGHT = 18;
}

Cherry::Cherry() : Structure({ WIDTH, HEIGHT, WIDTH }) {}

DynamicArray3D<Block> Cherry::build(uint32_t seed) const {
	DynamicArray3D<Block> blocks(size());
	tree::Rng rng{ seed | 1u };
	ivec2 c = getCenterPos();
	BlockID log = BlockID::DARK_LOG, leaf = BlockID::CHERRY_LEAVES;

	int height = rng.rangeI(9, 13);
	// Low trunk so the blossoms sit close to the ground.
	int trunkHeight = std::clamp(static_cast<int>(std::round(height * 0.32f)), 2, height - 5);
	float rx = rng.range(3.8f, 4.8f); // broad
	float ry = rng.range(2.6f, 3.4f); // thick, but flatter than it is wide
	int crownY = trunkHeight + static_cast<int>(std::round(ry * 0.7f));
	float startAngle = rng.unit() * 2.0f * glm::pi<float>();

	tree::branch(blocks, vec3(c.x, 0, c.y), vec3(c.x, trunkHeight, c.y), log);

	// Branches diverge from the trunk top up into the crown so the canopy rests on wood.
	int mains = rng.rangeI(3, 5);
	for (int i = 0; i < mains; ++i) {
		float angle = startAngle + i * tree::GOLDEN_ANGLE + rng.range(-0.2f, 0.2f);
		float dist = rx * rng.range(0.5f, 0.85f);
		int tipY = crownY + rng.rangeI(-1, 1);
		vec3 tip{ c.x + std::cos(angle) * dist, static_cast<float>(tipY), c.y + std::sin(angle) * dist };
		tree::branch(blocks, vec3(c.x, trunkHeight, c.y), tip, log);
	}

	// One broad, dense, flattened mass of blossoms.
	tree::leafEllipsoid(blocks, vec3(c.x, static_cast<float>(crownY), c.y), rx, ry, leaf);

	// Irregular lumps around the rim so the outline isn't a clean ellipse.
	int bumps = rng.rangeI(3, 6);
	for (int i = 0; i < bumps; ++i) {
		float angle = startAngle + i * tree::GOLDEN_ANGLE + rng.range(-0.3f, 0.3f);
		float d = rx * rng.range(0.7f, 1.0f);
		float by = crownY + rng.range(-1.0f, 1.0f);
		vec3 bump{ c.x + std::cos(angle) * d, by, c.y + std::sin(angle) * d };
		tree::leafBlobFlat(blocks, bump, rng.range(1.8f, 2.5f), leaf);
	}

	return blocks;
}

bool Cherry::isValidPos(ivec3 centerPos, BiomeID biomeID) const {
	const Biome& biome = g_worldGenerator.biomeMap().getBiome(biomeID);
	BlockID blockID = biome.getBlock(centerPos + Dir3D::to_ivec3(Dir3D::DOWN), 0).id;
	return centerPos.y >= Const::SEA_LEVEL && (blockID == +BlockID::GRASS || blockID == +BlockID::DIRT);
}
