#include "BulkEdit.h"

#include "Generator/Noise/OctavePerlin.h"

#include <random>
#include <cmath>

namespace BulkEdit {

std::vector<BlockEdit> smoothSphere(ivec3 center, int radius, Block block) {
	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_real_distribution<float> dist(0.f, 1.f);

	// Random sampling offset so every call gets a different irregular shape.
	const dvec3 noiseOffset{ dist(rng) * 256.0, dist(rng) * 256.0, dist(rng) * 256.0 };
	// How far the radius is allowed to wobble (size of the lobes).
	const float lobeAmplitude = 0.2f;
	// Smooth lobes plus a touch of surface roughness
	OctavePerlin perlin{ 2, 0.5, 0.09 };

	const float innerR = float(radius) * (1.f - lobeAmplitude) - 1.f;
	const float outerR = float(radius) * (1.f + lobeAmplitude) + 1.f;
	const int reach = int(std::ceil(outerR));

	std::vector<BlockEdit> edits;
	for (int x = -reach; x <= reach; ++x) {
		for (int y = -reach; y <= reach; ++y) {
			for (int z = -reach; z <= reach; ++z) {
				float dist3 = std::sqrt(float(x * x + y * y + z * z));
				if (dist3 <= innerR) { // deep interior: always affected
					edits.push_back({ center + ivec3{ x, y, z }, block });
					continue;
				}
				if (dist3 >= outerR) // beyond the largest possible lobe: never affected
					continue;
				// Sample coherent noise on the block's 3D position
				dvec3 p = noiseOffset + dvec3{ x, y, z };
				double n = (perlin.getNoise({ p.x, p.y })
				          + perlin.getNoise({ p.y, p.z })
				          + perlin.getNoise({ p.z, p.x })) / 3.0;
				float effRadius = float(radius) * (1.f + lobeAmplitude * float(n));

				// Rim with ragged edge: blocks well inside are always affected, blocks straddling the
				// boundary are affected with falling probability.
				float edge = effRadius - dist3;
				if (edge >= 1.f || (edge > -1.f && dist(rng) < (edge + 1.f) * 0.5f))
					edits.push_back({ center + ivec3{ x, y, z }, block });
			}
		}
	}
	return edits;
}

}
