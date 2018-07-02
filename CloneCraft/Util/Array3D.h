#pragma once

#include "GlmCommon.h"

template<typename T>
class Array3D
{
private:
	T *m_array{ nullptr };
	ivec3 m_size;

public:
	Array3D(ivec3 size) : m_size{ size }
	{
		m_array = new T[m_size.x * m_size.y * m_size.z];
	}

	Array3D(const Array3D& a) : Array3D{ a.m_size }
	{
		for (int i = 0; i < m_size.x * m_size.y * m_size.z; ++i)
			m_array[i] = a.m_array[i];
	}

	~Array3D()
	{
		delete[] m_array;
	}

	T& at(ivec3 index)
	{
		return m_array[(index.x * m_size.y + index.y) * m_size.z + index.z];
	}

	const T& at(ivec3 index) const
	{
		return m_array[(index.x * m_size.y + index.y) * m_size.z + index.z];
	}

	ivec3 size() const
	{
		return m_size;
	}
};