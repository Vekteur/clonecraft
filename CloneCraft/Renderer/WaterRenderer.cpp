#include "WaterRenderer.h"

#include "ResManager.h"
#include "Logger.h"
#include "ChunkMap.h"

#include <filesystem>
#include <WorldConstants.h>

namespace fs = std::filesystem;

WaterRenderer::WaterRenderer(ivec2 windowSize) {
	m_shader.loadFromFile("Resources/Shaders/water.vs", "Resources/Shaders/water.frag");

	getShader().use().set("distance", ChunkMap::SIDE);
	getShader().use().set("simple", simple);
	getShader().use().set("reflectionTexture", 0);
	getShader().use().set("refractionTexture", 1);
	getShader().use().set("dudvMap", 2);

	onChangedSize(windowSize);

	if (simple)
		glBlendFunc(GL_ZERO, GL_SRC_COLOR);
}

void WaterRenderer::reflectCamera(const DefaultRenderer& defaultRenderer, Camera& camera, ivec2 windowSize) {
	vec3 pos = camera.getPosition();
	camera.setPosition({ pos.x, 2 * Const::SEA_LEVEL - pos.y , pos.z });
	camera.invertPitch();
	camera.update(windowSize);

	defaultRenderer.getShader().use().set("view", camera.getViewMatrix());
	defaultRenderer.getShader().use().set("projection", camera.getProjMatrix());
	getShader().use().set("view", camera.getViewMatrix());
	getShader().use().set("projection", camera.getProjMatrix());
	getShader().use().set("reflectionTexture", 0);
}

void WaterRenderer::prepare(std::function<void()> renderFunc, std::function<void()> clearFunc,
	const DefaultRenderer& defaultRenderer, Camera& camera, ivec2 windowSize) {
	if (simple)
		return;
	reflectCamera(defaultRenderer, camera, windowSize);

	defaultRenderer.getShader().use().set("clipPlane", vec4(0, 1, 0, -Const::SEA_LEVEL));
	reflectionTexture.setActive(true);
	clearFunc();
	renderFunc();
	reflectionTexture.display();

	reflectCamera(defaultRenderer, camera, windowSize);

	defaultRenderer.getShader().use().set("clipPlane", vec4(0, -1, 0, Const::SEA_LEVEL));
	refractionTexture.setActive(true);
	clearFunc();
	renderFunc();
	refractionTexture.display();
	// Disable clip plane
	defaultRenderer.getShader().use().set("clipPlane", vec4(0, -1, 0, 10000));
}

void WaterRenderer::render(const WaterMesh& mesh) const {
	if (mesh.indicesNb != 0) {
		m_shader.use();
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		
		if (!simple) {
			glActiveTexture(GL_TEXTURE0);
			sf::Texture::bind(&reflectionTexture.getTexture());
			glActiveTexture(GL_TEXTURE1);
			sf::Texture::bind(&refractionTexture.getTexture());
			glActiveTexture(GL_TEXTURE2);
			sf::Texture::bind(&dudvMap);
		}
		mesh.draw();
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
}

void WaterRenderer::onChangedSize(ivec2 windowSize) {
	if (simple)
		return;
	sf::ContextSettings settings;
	settings.majorVersion = 4;
	settings.minorVersion = 3;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	if (!reflectionTexture.create(windowSize.x, windowSize.y, settings)) {
		LOG(Level::ERROR) << "Could not create reflection texture" << std::endl;
	}
	if (!refractionTexture.create(windowSize.x, windowSize.y, settings)) {
		LOG(Level::ERROR) << "Could not create refraction texture" << std::endl;
	}
	if (!dudvMap.loadFromFile("Resources/Textures/Water/dudvMap.png")) {
		LOG(Level::ERROR) << "Could not create DuDv Map texture" << std::endl;
	}
	dudvMap.setRepeated(true);
}

const Shader & WaterRenderer::getShader() const {
	return m_shader;
}
