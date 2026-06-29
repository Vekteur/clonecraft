#include "Maple.h"

#include "TreeShapes.h"
#include "Generator/WorldGenerator.h"
#include "Maths/Dir3D.h"

#include <algorithm>
#include <cmath>

namespace {
	constexpr int WIDTH = 13;
	constexpr int HEIGHT = 22;
}

Maple::Maple() : Structure({ WIDTH, HEIGHT, WIDTH }) {}

DynamicArray3D<Block> Maple::build(uint32_t seed) const {
	DynamicArray3D<Block> blocks(size());
	tree::Rng rng{ seed | 1u };
	ivec2 c = getCenterPos();
	BlockID log = BlockID::LOG, leaf = BlockID::MAPLE_LEAVES;

	int height = rng.rangeI(11, 20);
	int trunkHeight = std::clamp(static_cast<int>(std::round(height * 0.5f)), 4, height - 4);
	float maxRadius = rng.range(4.0f, 6.0f);

	int crownBottom = trunkHeight / 3 + rng.rangeI(0, 2);
	int crownTop = height;
	int crownHeight = std::max(1, crownTop - crownBottom);

	tree::branch(blocks, vec3(c.x, 0, c.y), vec3(c.x, trunkHeight, c.y), log);

	// Stacked disks following a half-ellipse: widest and flat at the base, holding their width then
	// curving over into a rounded top.
	for (int y = crownBottom; y <= crownTop; ++y) {
		float t = static_cast<float>(y - crownBottom) / crownHeight; // 0 bottom .. 1 top
		float radius = maxRadius * std::sqrt(std::max(0.0f, 1.0f - t * t));
		tree::leafDisk(blocks, ivec3(c.x, y, c.y), radius, leaf);
	}

	// Rounded knob capping the top so it doesn't taper to a single point.
	tree::leafDisk(blocks, ivec3(c.x, crownTop, c.y), 2.2f, leaf);
	tree::leafDisk(blocks, ivec3(c.x, crownTop + 1, c.y), 1.4f, leaf);

	// Side lobes low down to broaden the flat base and break up the silhouette.
	int lobes = rng.rangeI(2, 4);
	for (int i = 0; i < lobes; ++i) {
		float angle = rng.unit() * 2.0f * glm::pi<float>();
		float d = maxRadius * rng.range(0.5f, 0.8f);
		float ly = crownBottom + rng.range(0.0f, crownHeight * 0.4f);
		vec3 lobe{ c.x + std::cos(angle) * d, ly, c.y + std::sin(angle) * d };
		tree::leafBlob(blocks, lobe, rng.range(1.6f, 2.2f), leaf);
	}

	return blocks;
}

bool Maple::isValidPos(ivec3 centerPos, BiomeID biomeID) const {
	const Biome& biome = g_worldGenerator.biomeMap().getBiome(biomeID);
	BlockID blockID = biome.getBlock(centerPos + Dir3D::to_ivec3(Dir3D::DOWN), 0).id;
	return centerPos.y >= Const::SEA_LEVEL && (blockID == +BlockID::GRASS || blockID == +BlockID::DIRT);
}
