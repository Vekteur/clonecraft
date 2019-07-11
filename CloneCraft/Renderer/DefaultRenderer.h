#pragma once

#include "Maths/GlmCommon.h"
#include "glad/glad.h"
#include "ResManager/Shader.h"
#include "ResManager/TextureArray.h"
#include "World/Mesh.h"

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

