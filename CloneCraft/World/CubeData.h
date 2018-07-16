#pragma once

#include <array>
#include <glad\glad.h>
#include <GlmCommon.h>

#include "Dir3D.h"

template<typename T, int S>
using arr = std::array<T, S>;

namespace CubeData
{
	const arr<GLuint, 6> faceElementIndices{
		0, 1, 2,
		2, 3, 0
	};

	const arr<ivec2, 4> faceCoords{ {
		{ 0, 0 },
		{ 0, 1 },
		{ 1, 1 },
		{ 1, 0 }
	} };

	const arr<arr<vec3, 4>, Dir3D::SIZE> dirToFace = []() {

		const arr<vec3, 4> faceX0{ {
			{ 0, 0, 0 },
			{ 0, 1, 0 },
			{ 0, 1, 1 },
			{ 0, 0, 1 }
		} };

		const arr<vec3, 4> faceX1{ {
			{ 1, 0, 1 },
			{ 1, 1, 1 },
			{ 1, 1, 0 },
			{ 1, 0, 0 }
		} };

		const arr<vec3, 4> faceY0{ {
			{ 0, 0, 1 },
			{ 1, 0, 1 },
			{ 1, 0, 0 },
			{ 0, 0, 0 }
		} };

		const arr<vec3, 4> faceY1{ {
			{ 1, 1, 1 },
			{ 0, 1, 1 },
			{ 0, 1, 0 },
			{ 1, 1, 0 }
		} };

		const arr<vec3, 4> faceZ0{ {
			{ 1, 0, 0 },
			{ 1, 1, 0 },
			{ 0, 1, 0 },
			{ 0, 0, 0 }
		} };

		const arr<vec3, 4> faceZ1{ {
			{ 0, 0, 1 },
			{ 0, 1, 1 },
			{ 1, 1, 1 },
			{ 1, 0, 1 }
		} };

		return arr<arr<vec3, 4>, Dir3D::SIZE>{ faceY1, faceX1, faceZ1, faceY0, faceX0, faceZ0 };
	}();
}