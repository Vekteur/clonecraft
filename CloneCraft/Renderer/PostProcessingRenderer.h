#pragma once

#include "Shader.h"
#include "Mesh.h"

#include <functional>

#include <SFML/Graphics.hpp>

class PostProcessingRenderer {
public:
	PostProcessingRenderer(ivec2 windowSize);
	~PostProcessingRenderer();

	void prepare(std::function<void()> renderFunc, std::function<void()> clearFunc);
	void render();
	void onChangedSize(ivec2 windowSize);
	const Shader& getShader() const;

private:
	Shader m_shader;
	PostProcessingMesh m_mesh;
	sf::RenderTexture renderTexture;
};