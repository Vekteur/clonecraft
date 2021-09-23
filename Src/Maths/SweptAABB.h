#pragma once

#include <vector>
#include <limits>
#include <tuple>

#include <Maths/GlmCommon.h>
#include <Maths/AABB.h>

std::tuple<Box, ivec3> getBroadphaseBox(const Box& hitbox, const vec3& shift);
float sweptAABB(const Box& b, const vec3& v, const Box& b2, float time, vec3& next_v);
float sweptAABBonList(const Box& b, vec3 v, const std::vector<Box>& bs, float time, vec3& next_v);
vec3 repeatedSweptAABB(Box b, vec3 v, const std::vector<Box>& bs, float time);

