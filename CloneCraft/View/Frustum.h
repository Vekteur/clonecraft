#pragma once

#include <array>
#include <Dir3D.h>
#include <GlmCommon.h>

struct Plane : vec4 {
	vec3 norm() const {
		return { x, y, z };
	}
	bool contains(vec3 point) const {
		return glm::dot(norm(), point) >= -w;
	}
	void normalize() {
		*this /= glm::length(norm());
	}
};

struct Box {
	vec3 pos, size;
	vec3 firstPointCrossed(vec3 norm) {
		vec3 point = pos;
		for (int i = 0; i < 3; ++i) {
			if (norm[i] < 0.f)
				point[i] += size[i];
		}
		return point;
	}

	vec3 lastPointCrossed(vec3 norm) {
		vec3 point = pos;
		for (int i = 0; i < 3; ++i) {
			if (norm[i] > 0.f)
				point[i] += size[i];
		}
		return point;
	}
};

class Frustum {
public:
	Frustum() = default;
	Frustum(mat4 projView);

	bool isBoxOutside(Box box) const;

private:
	static const std::array<Dir3D::Dir, Dir3D::SIZE> planeDirs;
	std::array<Plane, Dir3D::SIZE> planes;
};

