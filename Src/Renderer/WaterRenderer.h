#pragma once

#include "Maths/GlmCommon.h"
#include "glad/glad.h"
#include "Renderer/Renderer.h"
#include "World/Mesh.h"
#include "DefaultRenderer.h"
#include "View/Camera.h"

#include <functional>
#include <SFML/Graphics.hpp>

class Section;

class WaterRenderer : public Renderer {
public:
	WaterRenderer(ivec2 windowSize, const Camera& camera, const DayCycle& dayCycle);

	void prepare(std::function<void(bool refractionPass)> renderFunc, std::function<void()> clearFunc,
		const DefaultRenderer& defaultRenderer, Camera& camera, ivec2 windowSize);
	void render(const Section& section) const override;
	void render(const WaterMesh& mesh) const;
	void update(sf::Time dt) override;
	void setUnderwater(bool underwater);
	void onChangedSize(ivec2 windowSize);
	sf::RenderTexture& getRefractionTexture();

private:
	void reflectCamera(const DefaultRenderer& defaultRenderer, Camera& camera, ivec2 windowSize);

	const bool simple = false;
	bool m_underwater = false;
	float m_moveOffset = 0;
	sf::RenderTexture reflectionTexture;
	sf::RenderTexture refractionTexture;
	sf::Texture dudvMap;
};

