#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

using glm::vec2;
using glm::vec3;
using glm::vec4;

using glm::ivec2;
using glm::ivec3;
using glm::ivec4;

using glm::dvec2;
using glm::dvec3;
using glm::dvec4;

using glm::mat3;
using glm::mat4;

std::ostream& operator <<(std::ostream& out, const vec2& vec);
std::ostream& operator <<(std::ostream& out, const vec3& vec);
std::ostream& operator <<(std::ostream& out, const vec4& vec);
std::ostream& operator <<(std::ostream& out, const mat4& vec);

std::ostream& operator <<(std::ostream& out, const ivec2& vec);
std::ostream& operator <<(std::ostream& out, const ivec3& vec);
std::ostream& operator <<(std::ostream& out, const ivec4& vec);

struct Comp_ivec2 {
    size_t operator()(const ivec2& vec) const {
        return std::hash<int>()(vec.x) ^ (std::hash<int>()(vec.y) << 1);
    }
    bool operator()(const ivec2& a, const ivec2& b) const {
        return a.x == b.x && a.y == b.y;
    }
};

struct Comp_ivec3 {
    size_t operator()(ivec3 vec) const {
        return std::hash<int>()(vec.x) ^ (std::hash<int>()(vec.y) << 1) ^ (std::hash<int>()(vec.z) << 2);
    }
    bool operator()(ivec3 a, ivec3 b) const { return a.x == b.x && a.y == b.y && a.z == b.z; }
};
