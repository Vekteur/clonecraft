#pragma once

#include "GlmCommon.h"
#include "glad/glad.h"
#include "Shader.h"
#include "Mesh.h"
#include "DefaultRenderer.h"
#include "Camera.h"

#include <functional>
#include <SFML/Graphics.hpp>

class WaterRenderer {
public:
	WaterRenderer(ivec2 windowSize);

	void prepare(std::function<void()> renderFunc, std::function<void()> clearFunc,
		const DefaultRenderer& defaultRenderer, Camera& camera, ivec2 windowSize);
	void render(const WaterMesh& mesh) const;
	void onChangedSize(ivec2 windowSize);
	const Shader& getShader() const;

private:
	void reflectCamera(const DefaultRenderer& defaultRenderer, Camera& camera, ivec2 windowSize);

	const bool simple = false;
	Shader m_shader;
	sf::RenderTexture reflectionTexture;
	sf::RenderTexture refractionTexture;
	sf::Texture dudvMap;
};

