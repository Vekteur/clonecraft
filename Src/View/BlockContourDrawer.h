#pragma once

#include <optional>

#include <glad/glad.h>

#include "Maths/GlmCommon.h"
#include "ResManager/Shader.h"

class BlockContourDrawer {
public:
	BlockContourDrawer();
	~BlockContourDrawer();

	BlockContourDrawer(const BlockContourDrawer&) = delete;
	BlockContourDrawer& operator=(const BlockContourDrawer&) = delete;

	void render(std::optional<ivec3> target, const mat4& view, const mat4& projection);

private:
	void buildMesh();

	Shader m_shader;
	GLuint m_VAO = 0, m_VBO = 0, m_EBO = 0;
};
