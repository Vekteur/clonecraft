#pragma once

#include "ResManager/Shader.h"
#include "World/Mesh.h"

#include <functional>
#include <memory>

#include <SFML/Graphics.hpp>

class DayCycle;

class PostProcessingRenderer {
public:
	PostProcessingRenderer(ivec2 windowSize, const DayCycle& dayCycle);

	void prepare(std::function<void()> renderFunc, std::function<void()> clearFunc);
	void render();
	void update(sf::Time dt);
	void setUnderwater(bool underwater);
	void setInLava(bool inLava);
	void onChangedSize(ivec2 windowSize);
	const Shader& getShader() const;
	sf::RenderTexture& getRenderTexture();

private:
	const DayCycle& m_dayCycle;
	bool m_underwater = false;
	bool m_inLava = false;
	Shader m_shader;
	PostProcessingMesh m_mesh;
	sf::RenderTexture m_renderTexture;
};