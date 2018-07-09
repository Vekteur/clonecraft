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

extern const ivec3 OUT_OF_CHUNK;
