#pragma once

#include "GlmCommon.h"
#include "glad/glad.h"
#include "Shader.h"
#include "TextureArray.h"
#include "Mesh.h"

class DefaultRenderer {
public:
	DefaultRenderer(TextureArray* p_texArray);

	void render(const DefaultMesh& mesh);

private:
	Shader m_shader;
	TextureArray* p_texArray;
};

