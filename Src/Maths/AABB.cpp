#include "AABB.h"

bool is_disjoint(Interval i1, Interval i2) {
	return i1.b < i2.a || i2.b < i1.a;
}

bool aabb_check(const Box& b1, const Box& b2) {
	for (int i = 0; i < 3; ++i) {
		if (!is_disjoint({ b1.pos[i], b1.pos[i] + b1.size[i] }, { b2.pos[i], b2.pos[i] + b2.size[i] }))
			return true;
	}
	return false;
}

std::ostream& operator<<(std::ostream& out, const Box& b) {
	out << b.pos << " " << b.size << std::endl;
	return out;
}
