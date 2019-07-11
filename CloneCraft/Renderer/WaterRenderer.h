#pragma once

#include "Maths/GlmCommon.h"
#include "glad/glad.h"
#include "ResManager/Shader.h"
#include "World/Mesh.h"
#include "DefaultRenderer.h"
#include "View/Camera.h"

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
	sf::RenderTexture& getRefractionTexture();

private:
	void reflectCamera(const DefaultRenderer& defaultRenderer, Camera& camera, ivec2 windowSize);

	const bool simple = false;
	Shader m_shader;
	sf::RenderTexture reflectionTexture;
	sf::RenderTexture refractionTexture;
	sf::Texture dudvMap;
};

