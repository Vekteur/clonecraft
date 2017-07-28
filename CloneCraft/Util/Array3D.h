#pragma once

#include <GlmCommon.h>

template<typename T>
class Array3D
{
private:
	T ***m_array;
	ivec3 m_size;

public:
	Array3D(ivec3 size) : m_size{ size }
	{
		m_array = new T**[m_size.x];

		for (int i = 0; i < m_size.x; i++)
		{
			m_array[i] = new T*[m_size.y];

			for (int j = 0; j < m_size.y; j++)
			{
				m_array[i][j] = new T[m_size.z];
			}
		}
	}

	Array3D(const Array3D& a) : Array3D{ a.m_size }
	{
		for (int i = 0; i < m_size.x; i++)
			for (int j = 0; j < m_size.y; j++)
				for (int k = 0; k < m_size.z; k++)
					m_array[i][j][k] = a.m_array[i][j][k];
	}

	~Array3D()
	{
		for (int i = 0; i < m_size.x; i++)
		{
			for (int j = 0; j < m_size.y; j++)
			{
				delete[] m_array[i][j];
			}
			delete[] m_array[i];
		}
		delete[] m_array;
	}

	T& at(ivec3 index)
	{
		return m_array[index.x][index.y][index.z];
	}

	const T& at(ivec3 index) const
	{
		return m_array[index.x][index.y][index.z];
	}

	vec3 size() const
	{
		return m_size;
	}
};

