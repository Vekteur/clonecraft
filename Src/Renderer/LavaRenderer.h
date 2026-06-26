#pragma once

#include "Maths/GlmCommon.h"
#include "glad/glad.h"
#include "Renderer/Renderer.h"
#include "World/Mesh.h"

class Section;

// Draws lava with an animated, self-lit shader. Lava is opaque, so it is rendered in the same pass
// as the terrain (no reflection/refraction needed); the molten look is fully procedural in the shader.
class LavaRenderer : public Renderer {
public:
	LavaRenderer(const Camera& camera, const DayCycle& dayCycle);

	void render(const Section& section) const override;
	void render(const LavaMesh& mesh) const;
	void update(sf::Time dt) override;

private:
	float m_lavaTime{ 0.f };
};
