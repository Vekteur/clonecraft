#pragma once

#include "Maths/GlmCommon.h"

#include <array>

template<typename T, int S1, int S2, int S3>
class Array3D {
private:
	std::array<std::array<std::array<T, S3>, S2>, S1> m_array;

public:
	T& at(ivec3 index) {
		return m_array[index.x][index.y][index.z];
	}

	const T& at(ivec3 index) const {
		return m_array[index.x][index.y][index.z];
	}

	ivec3 size() const {
		return { S1, S2, S3 };
	}
};