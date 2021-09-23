#pragma once

#include <Maths/GlmCommon.h>

struct Box {
	vec3 pos, size;
};

struct Interval {
	float a, b;
};

bool is_disjoint(Interval i1, Interval i2);
bool aabb_check(const Box& b1, const Box& b2);

std::ostream& operator <<(std::ostream& out, const Box& b);