#pragma once

#include "GlmCommon.h"
#include "glad/glad.h"
#include "Shader.h"
#include "TextureArray.h"
#include "Mesh.h"

class DefaultRenderer {
public:
	DefaultRenderer();

	void render(const DefaultMesh& mesh) const;
	TextureArray& getTextureArray();
	const Shader& getShader() const;

private:
	Shader m_shader;
	TextureArray texArray;
};

