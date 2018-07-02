#include "GlmCommon.h"

std::ostream& operator <<(std::ostream& out, const vec3& vec) {
	for (int i = 0; i < 3; ++i)
		out << vec[i] << ' ';
	return out;
}

std::ostream& operator <<(std::ostream& out, const vec4& vec) {
	for (int i = 0; i < 4; ++i)
		out << vec[i] << ' ';
	return out;
}

std::ostream& operator <<(std::ostream& out, const mat4& mat) {
	for (int i = 0; i < 4; ++i)
		out << mat[i] << '\n';
	return out;
}