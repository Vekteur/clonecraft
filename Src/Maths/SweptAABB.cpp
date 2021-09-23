#include "SweptAABB.h"

int argmin(vec3 v) {
	int r = 0;
	for (int i = 1; i < 3; ++i)
		if (v[i] < v[r])
			r = i;
	return r;
}

float min(vec3 v) {
	return v[argmin(v)];
}

int argmax(vec3 v) {
	int r = 0;
	for (int i = 1; i < 3; ++i)
		if (v[i] > v[r])
			r = i;
	return r;
}

float max(vec3 v) {
	return v[argmax(v)];
}

bool is_disjoint_or_touching(Interval i1, Interval i2) {
	return i1.b <= i2.a || i2.b <= i1.a;
}

std::tuple<Box, ivec3> getBroadphaseBox(const Box& hitbox, const vec3& shift) {
	Box b = hitbox;
	b.size += abs(shift);
	ivec3 dirs;
	// dirs[i] is 1 if the hitbox is towards infinity on the ith axis of the broadphase box, -1 else
	for (int i = 0; i < 3; ++i) {
		if (shift[i] >= 0) {
			dirs[i] = 1;
		} else {
			dirs[i] = -1;
			b.pos[i] += shift[i];
		}
	}
	return { b, dirs };
}


float sweptAABB(const Box& b, const vec3& v, const Box& b2, float time, vec3& next_v) {
	vec3 entryTimes, exitTimes;

	bool disjoint = false;
	for (int i = 0; i < 3; ++i) {
		if (v[i] == 0.f) {
			entryTimes[i] = -std::numeric_limits<float>::infinity();
			exitTimes[i] = std::numeric_limits<float>::infinity();
			if (is_disjoint_or_touching({ b.pos[i], b.pos[i] + b.size[i] }, { b2.pos[i], b2.pos[i] + b2.size[i] }))
				disjoint = true;
			continue;
		}
		entryTimes[i] = b2.pos[i] - (b.pos[i] + b.size[i]);
		exitTimes[i] = (b2.pos[i] + b2.size[i]) - b.pos[i];
		if (v[i] < 0) {
			std::swap(entryTimes[i], exitTimes[i]);
		}
		entryTimes[i] /= v[i];
		exitTimes[i] /= v[i];
	}
	
	float entryTime = max(entryTimes);
	float exitTime = min(exitTimes);

	// Set `|| 0.f <= entryTime` to allow moving during suffocation
	if (disjoint || entryTime > exitTime || time <= entryTime) {
		next_v = vec3();
		return time;
	}
	
	next_v = v;
	next_v[argmax(entryTimes)] = 0.f;

	return entryTime;
}

float sweptAABBonList(const Box& b, vec3 v, const std::vector<Box>& bs, float time, vec3& next_v) {
	float min_time = time;
	next_v = vec3();
	for (const Box& b2 : bs) {
		vec3 curr_next_v;
		float box_time = sweptAABB(b, v, b2, time, curr_next_v);
		// It is very important to use an epsilon here so that the last boxes in the loop will be
		// prioritized for boxes that are at a distance close to 0
		const float epsilon = 1e-8f;
		if (box_time - epsilon < min_time) {
			min_time = box_time;
			next_v = curr_next_v;
		}
	}
	return min_time;
}

int sign(float v) {
	if (v == 0.f) return 0;
	else if (v < 0.f) return -1;
	return 1;
}

ivec3 sign(vec3 v) {
	ivec3 r;
	for (int i = 0; i < 3; ++i)
		r[i] = sign(v[i]);
	return r;
}

vec3 repeatedSweptAABB(Box b, vec3 v, const std::vector<Box>& bs, float time) {
	// After each step, the velocity is projected onto another plan
	// Thus, there can be at most 3 steps in 3D
	vec3 total_shift{};
	for (int repeat = 0; repeat < 3 && v != vec3(); ++repeat) {
		vec3 next_v;
		float collision_time = sweptAABBonList(b, v, bs, time, next_v);
		collision_time = std::max(0.f, collision_time);
		vec3 shift = v * collision_time;

		b.pos += shift;
		total_shift += shift;
		time -= collision_time;
		v = next_v;
	}
	return total_shift;
}