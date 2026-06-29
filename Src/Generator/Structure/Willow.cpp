#include "Willow.h"

#include "TreeShapes.h"
#include "Generator/WorldGenerator.h"
#include "Maths/Dir3D.h"

#include <algorithm>
#include <cmath>

namespace {
	constexpr int WIDTH = 17; // wide, drooping crown
	constexpr int HEIGHT = 18;
}

Willow::Willow() : Structure({ WIDTH, HEIGHT, WIDTH }) {}

DynamicArray3D<Block> Willow::build(uint32_t seed) const {
	DynamicArray3D<Block> blocks(size());
	tree::Rng rng{ seed | 1u };
	ivec2 c = getCenterPos();
	BlockID log = BlockID::LOG, leaf = BlockID::WILLOW_LEAVES;

	int height = rng.rangeI(9, 15);
	int trunkHeight = std::clamp(static_cast<int>(std::round(height * 0.6f)), 5, height - 2);
	float reach = rng.range(3.5f, 5.0f);
	float crownR = rng.range(2.5f, 3.2f);
	float startAngle = rng.unit() * 2.0f * glm::pi<float>();

	tree::branch(blocks, vec3(c.x, 0, c.y), vec3(c.x, trunkHeight, c.y), log);

	// Central crown over the trunk.
	tree::leafBlob(blocks, vec3(c.x, static_cast<float>(trunkHeight), c.y), crownR, leaf);

	// Branches arch outward almost flat, each tipped with a small clump and drooping strands.
	int branchCount = rng.rangeI(5, 8);
	for (int i = 0; i < branchCount; ++i) {
		float angle = startAngle + i * tree::GOLDEN_ANGLE + rng.range(-0.2f, 0.2f);
		float dist = reach * rng.range(0.7f, 1.0f);
		int tipY = trunkHeight + rng.rangeI(-1, 1);
		vec3 tip{ c.x + std::cos(angle) * dist, static_cast<float>(tipY), c.y + std::sin(angle) * dist };

		tree::branch(blocks, vec3(c.x, trunkHeight - 1, c.y), tip, log);
		tree::leafBlob(blocks, tip, rng.range(1.8f, 2.5f), leaf);

		int fronds = rng.rangeI(2, 4);
		for (int f = 0; f < fronds; ++f) {
			float a = rng.unit() * 2.0f * glm::pi<float>();
			float d = rng.range(0.0f, 1.6f);
			ivec3 top = ivec3(glm::round(vec3(tip.x + std::cos(a) * d, tip.y - 1.0f, tip.z + std::sin(a) * d)));
			tree::leafStrand(blocks, top, rng.rangeI(3, 6), leaf);
		}
	}

	// A ring of extra strands hanging straight off the crown. Kept within the crown radius so each
	// strand starts inside the leaves rather than floating.
	int ringStrands = rng.rangeI(6, 10);
	for (int i = 0; i < ringStrands; ++i) {
		float angle = startAngle + i * tree::GOLDEN_ANGLE;
		float d = rng.range(1.0f, crownR);
		ivec3 top = ivec3(glm::round(vec3(c.x + std::cos(angle) * d, static_cast<float>(trunkHeight), c.y + std::sin(angle) * d)));
		tree::leafStrand(blocks, top, rng.rangeI(2, 5), leaf);
	}

	return blocks;
}

bool Willow::isValidPos(ivec3 centerPos, BiomeID biomeID) const {
	const Biome& biome = g_worldGenerator.biomeMap().getBiome(biomeID);
	BlockID blockID = biome.getBlock(centerPos + Dir3D::to_ivec3(Dir3D::DOWN), 0).id;
	return centerPos.y >= Const::SEA_LEVEL && (blockID == +BlockID::GRASS || blockID == +BlockID::DIRT);
}
