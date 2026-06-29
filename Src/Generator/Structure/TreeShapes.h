#pragma once

#include "Util/DynamicArray3D.h"
#include "Block/Block.h"
#include "Maths/GlmCommon.h"
#include "Maths/LineBlockFinder.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

// Shared primitives for procedural trees. A tree builds into a local voxel buffer (so generation
// stays thread-safe) using a seed derived from its world position, so every chunk that overlaps it
// rebuilds the exact same tree.
namespace tree {
	// Golden angle: successive branches fan out evenly around the trunk instead of clumping.
	constexpr float GOLDEN_ANGLE = 2.39996323f;

	struct Rng {
		uint32_t state;
		uint32_t next() {
			state ^= state << 13;
			state ^= state >> 17;
			state ^= state << 5;
			return state;
		}
		float unit() { return (next() >> 8) * (1.0f / 16777216.0f); } // [0, 1)
		float range(float lo, float hi) { return lo + unit() * (hi - lo); }
		int rangeI(int lo, int hi) { return lo + static_cast<int>(next() % static_cast<uint32_t>(hi - lo + 1)); }
	};

	inline void setLog(DynamicArray3D<Block>& b, ivec3 p, BlockID log) {
		if (b.isValidIndex(p))
			b.at(p) = log;
	}

	inline void setLeaf(DynamicArray3D<Block>& b, ivec3 p, BlockID leaf) {
		if (b.isValidIndex(p) && b.at(p).id == +BlockID::AIR)
			b.at(p) = leaf;
	}

	// Line of logs between two points (used for trunk and branches).
	inline void branch(DynamicArray3D<Block>& b, vec3 from, vec3 to, BlockID log) {
		vec3 d = to - from;
		float distance = glm::length(d);
		LineBlockFinder lbf{ from, glm::normalize(d) };
		while (lbf.getDistance() <= distance) {
			ivec3 iterPos = lbf.next();
			setLog(b, iterPos, log);
		}
	}

	// Flat rounded disk of leaves. The +0.4 bias rounds off the corners so it reads as a circle.
	inline void leafDisk(DynamicArray3D<Block>& b, ivec3 c, float radius, BlockID leaf) {
		if (radius < 0.5f) {
			setLeaf(b, c, leaf);
			return;
		}
		int r = static_cast<int>(std::ceil(radius));
		for (int dx = -r; dx <= r; ++dx)
			for (int dz = -r; dz <= r; ++dz) {
				float ex = std::abs(dx) + 0.4f, ez = std::abs(dz) + 0.4f;
				if (ex * ex + ez * ez <= radius * radius)
					setLeaf(b, c + ivec3(dx, 0, dz), leaf);
			}
	}

	// Rounded clump of leaves: a few stacked disks, widest in the middle.
	inline void leafBlob(DynamicArray3D<Block>& b, vec3 center, float radius, BlockID leaf) {
		ivec3 c = ivec3(glm::round(center));
		leafDisk(b, c + ivec3(0, -1, 0), radius - 1.2f, leaf);
		leafDisk(b, c + ivec3(0, 0, 0), radius, leaf);
		leafDisk(b, c + ivec3(0, 1, 0), radius - 0.4f, leaf);
		leafDisk(b, c + ivec3(0, 2, 0), radius - 1.5f, leaf);
	}

	// Flatter, wider clump for spreading crowns (cherry).
	inline void leafBlobFlat(DynamicArray3D<Block>& b, vec3 center, float radius, BlockID leaf) {
		ivec3 c = ivec3(glm::round(center));
		leafDisk(b, c + ivec3(0, -1, 0), radius - 1.0f, leaf);
		leafDisk(b, c + ivec3(0, 0, 0), radius, leaf);
		leafDisk(b, c + ivec3(0, 1, 0), radius - 1.3f, leaf);
	}

	// Dense leaf ellipsoid (rounded crown). Wider than tall when rx > ry.
	inline void leafEllipsoid(DynamicArray3D<Block>& b, vec3 center, float rx, float ry, BlockID leaf) {
		int Rx = static_cast<int>(std::ceil(rx)), Ry = static_cast<int>(std::ceil(ry));
		ivec3 c = ivec3(glm::round(center));
		for (int dx = -Rx; dx <= Rx; ++dx)
			for (int dy = -Ry; dy <= Ry; ++dy)
				for (int dz = -Rx; dz <= Rx; ++dz) {
					float ex = (std::abs(dx) + 0.3f) / rx;
					float ey = (std::abs(dy) + 0.3f) / ry;
					float ez = (std::abs(dz) + 0.3f) / rx;
					if (ex * ex + ey * ey + ez * ez <= 1.0f)
						setLeaf(b, c + ivec3(dx, dy, dz), leaf);
				}
	}

	// Hanging strand of leaves dropping straight down (willow fronds).
	inline void leafStrand(DynamicArray3D<Block>& b, ivec3 top, int length, BlockID leaf) {
		for (int i = 0; i < length; ++i)
			setLeaf(b, top - ivec3(0, i, 0), leaf);
	}

	// Tapered tongue of leaves from base out to tip, thinning to a point at the tip. Drawn as a
	// chain of flat disks so a tip set below the base reads as a drooping branch (conifer whorls).
	inline void leafLimb(DynamicArray3D<Block>& b, vec3 base, vec3 tip, float baseRadius, BlockID leaf) {
		vec3 d = tip - base;
		int steps = std::max(1, static_cast<int>(std::ceil(glm::length(d))));
		for (int i = 0; i <= steps; ++i) {
			float t = static_cast<float>(i) / steps;
			vec3 p = base + d * t;
			leafDisk(b, ivec3(glm::round(p)), baseRadius * (1.0f - t), leaf);
		}
	}
}
