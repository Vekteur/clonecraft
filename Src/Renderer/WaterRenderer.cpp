#include "WaterRenderer.h"

#include "Game/DayCycle.h"
#include "ResManager/ResManager.h"
#include "Util/Logger.h"
#include "View/Camera.h"
#include "World/ChunkMap.h"
#include "World/Section.h"

#include <cmath>
#include <filesystem>
#include <World/WorldConstants.h>

namespace fs = std::filesystem;

WaterRenderer::WaterRenderer(ivec2 windowSize, const Camera& camera, const DayCycle& dayCycle)
	: Renderer(camera, dayCycle) {
	m_shader.loadFromFile("Data/Shaders/water.vs", "Data/Shaders/water.frag");

	getShader().use().set("distance", ChunkMap::SIDE);
	getShader().use().set("simple", simple);
	getShader().use().set("reflectionTexture", 0);
	getShader().use().set("refractionTexture", 1);
	getShader().use().set("dudvMap", 2);

	onChangedSize(windowSize);
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

void WaterRenderer::prepare(std::function<void(bool refractionPass)> renderFunc, std::function<void()> clearFunc,
	const DefaultRenderer& defaultRenderer, Camera& camera, ivec2 windowSize) {
	if (!simple) {
		reflectCamera(defaultRenderer, camera, windowSize);

		defaultRenderer.getShader().use().set("clipPlane", vec4(0, 1, 0, -Const::SEA_LEVEL));
		reflectionTexture.setActive(true);
		clearFunc();
		renderFunc(false);
		reflectionTexture.display();

		reflectCamera(defaultRenderer, camera, windowSize);
	}

	defaultRenderer.getShader().use().set("clipPlane", vec4(0, -1, 0, 10000));
	refractionTexture.setActive(true);
	clearFunc();
	renderFunc(true);
	refractionTexture.display();
}

void WaterRenderer::render(const Section& section) const {
	render(section.getWaterMesh());
}

void WaterRenderer::render(const WaterMesh& mesh) const {
	if (mesh.indicesNb != 0) {
		m_shader.use();
		
		if (!simple) {
			glActiveTexture(GL_TEXTURE0);
			sf::Texture::bind(&reflectionTexture.getTexture());
		}
		glActiveTexture(GL_TEXTURE1);
		sf::Texture::bind(&refractionTexture.getTexture());
		glActiveTexture(GL_TEXTURE2);
		sf::Texture::bind(&dudvMap);
		mesh.draw();
	}
}

void WaterRenderer::update(sf::Time dt) {
	getShader().use().set("view", m_camera.getViewMatrix());
	getShader().use().set("projection", m_camera.getProjMatrix());
	getShader().use().set("skyColor", m_dayCycle.getSkyColor());
	getShader().use().set("cameraPosition", m_camera.getPosition());

	m_moveOffset = std::fmod(m_moveOffset + 0.02f * dt.asSeconds(), 1.f);
	getShader().use().set("moveOffset", m_moveOffset);

	getShader().use().set("underwater", m_underwater);
}

void WaterRenderer::setUnderwater(bool underwater) {
	m_underwater = underwater;
}

void WaterRenderer::onChangedSize(ivec2 windowSize) {
	sf::ContextSettings settings;
	settings.majorVersion = 4;
	settings.minorVersion = 3;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 0;
	if (!simple && !reflectionTexture.create(windowSize.x, windowSize.y, settings)) {
		LOG(Level::ERROR) << "Could not create reflection texture" << std::endl;
	}
	if (!refractionTexture.create(windowSize.x, windowSize.y, settings)) {
		LOG(Level::ERROR) << "Could not create refraction texture" << std::endl;
	}
	if (!dudvMap.loadFromFile("Data/Textures/Water/dudvMap.png")) {
		LOG(Level::ERROR) << "Could not create DuDv Map texture" << std::endl;
	}
	dudvMap.setRepeated(true);
}

sf::RenderTexture& WaterRenderer::getRefractionTexture() {
	return refractionTexture;
}
