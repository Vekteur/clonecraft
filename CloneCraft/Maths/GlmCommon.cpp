#include "GlmCommon.h"

template<typename T, int S>
std::ostream& operator <<(std::ostream& out, const T& vec) {
	out << '(';
	for (int i = 0; i < S - 1; ++i)
		out << vec[i] << ' ';
	out << vec[S - 1] << ')';
	return out;
}

std::ostream& operator <<(std::ostream& out, const vec2& vec) {
	return operator << <vec2, 2>(out, vec);
}

std::ostream& operator <<(std::ostream& out, const vec3& vec) {
	return operator << <vec3, 3>(out, vec);
}

std::ostream& operator <<(std::ostream& out, const vec4& vec) {
	return operator << <vec4, 4>(out, vec);
}

std::ostream& operator <<(std::ostream& out, const mat4& mat) {
	return operator << <mat4, 4>(out, mat);
}

std::ostream& operator <<(std::ostream& out, const ivec2& vec) {
	return operator << <ivec2, 2>(out, vec);
}

std::ostream& operator <<(std::ostream& out, const ivec3& vec) {
	return operator << <ivec3, 3>(out, vec);
}

std::ostream& operator <<(std::ostream& out, const ivec4& vec) {
	return operator << <ivec4, 4>(out, vec);
}

const ivec3 OUT_OF_CHUNK = ivec3(0, -1, 0);