#include "LavaRenderer.h"

#include "Game/DayCycle.h"
#include "View/Camera.h"
#include "World/ChunkMap.h"
#include "World/Section.h"

LavaRenderer::LavaRenderer(const Camera& camera, const DayCycle& dayCycle)
	: Renderer(camera, dayCycle) {
	m_shader.loadFromFile("Data/Shaders/lava.vs", "Data/Shaders/lava.frag");
	getShader().use().set("distance", ChunkMap::SIDE);
}

void LavaRenderer::render(const Section& section) const {
	render(section.getLavaMesh());
}

void LavaRenderer::update(sf::Time dt) {
	m_lavaTime = std::fmod(m_lavaTime + dt.asSeconds(), 3600.f);

	getShader().use().set("view", m_camera.getViewMatrix());
	getShader().use().set("projection", m_camera.getProjMatrix());
	getShader().use().set("skyColor", m_dayCycle.getSkyColor());
	getShader().use().set("time", m_lavaTime);
}

void LavaRenderer::render(const LavaMesh& mesh) const {
	if (mesh.indicesNb != 0) {
		m_shader.use();
		mesh.draw();
	}
}
