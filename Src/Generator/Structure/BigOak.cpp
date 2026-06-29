#include "BigOak.h"

#include "TreeShapes.h"
#include "Generator/WorldGenerator.h"
#include "Maths/Dir3D.h"

#include <algorithm>
#include <cmath>

namespace {
	constexpr int WIDTH = 15;
	constexpr int HEIGHT = 26;
}

BigOak::BigOak() : Structure({ WIDTH, HEIGHT, WIDTH }) {}

DynamicArray3D<Block> BigOak::build(uint32_t seed) const {
	DynamicArray3D<Block> blocks(size());
	tree::Rng rng{ seed | 1u };
	ivec2 center = getCenterPos();
	BlockID log = BlockID::LOG, leaf = BlockID::LEAVES;

	int height = rng.rangeI(10, 20);
	int trunkHeight = std::clamp(static_cast<int>(std::round(height * 0.62f)), 5, height - 2);
	float maxSpread = rng.range(2.5f, 3.5f);

	// Straight trunk.
	tree::branch(blocks, vec3(center.x, 0, center.y), vec3(center.x, trunkHeight, center.y), log);

	// Foliage clusters climb the upper trunk, each fed by a branch from the trunk. Lower clusters
	// reach further out, giving the crown a rounded silhouette that narrows toward the top.
	int clusterCount = 5 + height / 5 + rng.rangeI(0, 2);
	int clusterBottom = static_cast<int>(std::round(height * 0.45f));
	float startAngle = rng.unit() * 2.0f * glm::pi<float>();
	for (int i = 0; i < clusterCount; ++i) {
		float t = clusterCount == 1 ? 1.0f : static_cast<float>(i) / (clusterCount - 1);
		int cy = static_cast<int>(std::round(clusterBottom + t * (height - clusterBottom)));
		float spread = (1.0f - t) * maxSpread * rng.range(0.6f, 1.0f) + 0.6f;
		float angle = startAngle + i * tree::GOLDEN_ANGLE;
		vec3 clusterPos{ center.x + std::cos(angle) * spread, static_cast<float>(cy),
				center.y + std::sin(angle) * spread };

		int baseY = std::min(cy, trunkHeight);
		tree::branch(blocks, vec3(center.x, baseY, center.y), clusterPos, log);
		tree::leafBlob(blocks, clusterPos, rng.range(2.0f, 2.8f), leaf);
	}

	// Crown capping the trunk top.
	tree::leafBlob(blocks, vec3(center.x, static_cast<float>(height), center.y), rng.range(2.4f, 3.0f), leaf);

	return blocks;
}

bool BigOak::isValidPos(ivec3 centerPos, BiomeID biomeID) const {
	const Biome& biome = g_worldGenerator.biomeMap().getBiome(biomeID);
	BlockID blockID = biome.getBlock(centerPos + Dir3D::to_ivec3(Dir3D::DOWN), 0).id;
	return centerPos.y >= Const::SEA_LEVEL && (blockID == +BlockID::GRASS || blockID == +BlockID::DIRT);
}
