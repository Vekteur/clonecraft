#pragma once

#include "ResManager/Shader.h"

#include <SFML/System/Time.hpp>

class Section;
class Camera;
class DayCycle;

class Renderer {
public:
	Renderer(const Camera& camera, const DayCycle& dayCycle)
		: m_camera(camera), m_dayCycle(dayCycle) {}
	virtual ~Renderer() = default;

	virtual void render(const Section& section) const = 0;
	// Refreshes this renderer's shader uniforms for the new frame.
	virtual void update(sf::Time dt) = 0;
	const Shader& getShader() const { return m_shader; }

protected:
	Shader m_shader;
	const Camera& m_camera;
	const DayCycle& m_dayCycle;
};
