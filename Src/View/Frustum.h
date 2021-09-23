#pragma once

#include <array>

#include <Maths/Dir3D.h>
#include <Maths/GlmCommon.h>
#include <Maths/AABB.h>

struct Plane : vec4 {
	vec3 norm() const;
	bool contains(vec3 point) const;
	void normalize();
};

vec3 firstPointCrossed(const Box& box, vec3 norm);
vec3 lastPointCrossed(const Box& box, vec3 norm);

class Frustum {
public:
	Frustum() = default;
	Frustum(mat4 projView);

	bool isBoxOutside(Box box) const;

private:
	static const std::array<Dir3D::Dir, Dir3D::SIZE> planeDirs;
	std::array<Plane, Dir3D::SIZE> planes;
};

