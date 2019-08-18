#pragma once

#include "Maths/GlmCommon.h"

#include <vector>

template<typename T>
class DynamicArray3D {
private:
	ivec3 m_size;
	std::vector<T> m_array;

public:
	DynamicArray3D(ivec3 size) : m_size{ size }, m_array(size.x* size.y* size.z) { }

	T& at(ivec3 index) {
		return m_array[(index.x * m_size.y + index.y) * m_size.z + index.z];
	}

	const T& at(ivec3 index) const {
		return m_array[(index.x * m_size.y + index.y) * m_size.z + index.z];
	}

	ivec3 size() const {
		return m_size;
	}
};