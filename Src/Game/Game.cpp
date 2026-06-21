#include "Game.h"

#include <random>
#include <cmath>

#include "Generator/Noise/OctavePerlin.h"
#include "Maths/Converter.h"
#include "Maths/Dir3D.h"
#include "Util/DebugGL.h"
#include "Util/Logger.h"
#include "World/WorldConstants.h"

Game::Game(Window* const window)
	: m_player{ this }, p_window{ window },
	m_waterRenderer{ { p_window->size() } }, m_postProcessingRenderer{ { p_window->size() } } {

	ResManager::initBlockDatas(std::vector<TextureArray*>{ &m_defaultRenderer.getTextureArray() });

	unsigned int workerCount = ChunkMap::LOADING_WORKERS_COUNT;
	for (unsigned int i = 0; i < workerCount; ++i)
		m_workerThreads.emplace_back(&Game::runWorkerLoop, this);
	m_orchestratorThread = std::thread{ &Game::runOrchestratorLoop, this };
}

Game::~Game() {
	m_stopGeneratingThread = true;
	m_chunkMap.stop();
	m_orchestratorThread.join();
	for (std::thread& worker : m_workerThreads)
		worker.join();
	if (m_updatingThread.joinable())
		m_updatingThread.join();
}

void Game::runOrchestratorLoop() {
	// Keeps the sorted view list fresh and unloads far chunks
	// CPU-only; no OpenGL context needed here.
	while (!m_stopGeneratingThread) {
		m_chunkMap.refreshSelection(m_player.getCamera().getFrustum());
		m_chunkMap.unloadFarChunks();
		std::this_thread::sleep_for(std::chrono::milliseconds(8));
	}
}

void Game::runWorkerLoop() {
	// Pulls and runs load tasks (terrain generation and mesh building); no OpenGL context here.
	while (!m_stopGeneratingThread) {
		m_chunkMap.processNextTask();
	}
}

void Game::onChangedSize(ivec2 size) {
	glViewport(0, 0, size.x, size.y);
	p_window->setView(sf::View(sf::FloatRect(0.f, 0.f, static_cast<float>(size.x), static_cast<float>(size.y))));
	m_waterRenderer.onChangedSize(p_window->size());
	m_postProcessingRenderer.onChangedSize(p_window->size());
}

bool Game::canReloadBlocks() {
	return updatingThreadFinished;
}

void Game::reloadBlocksMeshes(const std::vector<ivec3>& blocks) {
	if (updatingThreadFinished) {
		updatingThreadFinished = false;
		if (m_updatingThread.joinable())
			m_updatingThread.join();
		m_updatingThread = std::thread{ [this, blocks]() {
			// CPU-only: rebuilds the affected sections' mesh data; the GPU upload happens
			// on the main thread in update(). No OpenGL context needed here.
			m_chunkMap.reloadBlocksMeshes(blocks);
			updatingThreadFinished = true;
		} };
	}
}

void Game::processKeyboard(sf::Time dt, Commands& commands) {
	if (commands.isActive(Command::FORWARD))
		m_player.move(Movement::FORWARD, dt);
	if (commands.isActive(Command::BACKWARD))
		m_player.move(Movement::BACKWARD, dt);
	if (commands.isActive(Command::LEFT))
		m_player.move(Movement::LEFT, dt);
	if (commands.isActive(Command::RIGHT))
		m_player.move(Movement::RIGHT, dt);
	if (commands.isActive(Command::UP))
		m_player.move(Movement::UP, dt);
	if (commands.isActive(Command::DOWN))
		m_player.move(Movement::DOWN, dt);
	if (commands.isActive(Command::SPRINT))
		m_player.setSprinting(true);
	if (commands.isActive(Command::EXPLOSION))
		fillSmoothSphere(15, +BlockID::AIR);
	if (commands.isActive(Command::HUGE_EXPLOSION))
		fillSmoothSphere(100, +BlockID::AIR);
	if (commands.isActive(Command::BRUSH))
		if (m_player.getPickedBlock().has_value())
			fillSmoothSphere(15, m_player.getPickedBlock().value());
	if (commands.isActive(Command::HUGE_BRUSH))
		if (m_player.getPickedBlock().has_value())
			fillSmoothSphere(100, m_player.getPickedBlock().value());
	if (commands.isActive(Command::TELEPORT))
		m_player.teleport();
	if (commands.isActive(Command::NEXT_GAMEMODE))
		m_player.nextGameMode();
}

std::vector<ivec3> Game::smoothSphere(ivec3 center, int radius) {
	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_real_distribution<float> dist(0.f, 1.f);

	// Random sampling offset so every explosion gets a different irregular shape.
	const dvec3 noiseOffset{ dist(rng) * 256.0, dist(rng) * 256.0, dist(rng) * 256.0 };
	// How far the crater radius is allowed to wobble (size of the lobes).
	const float lobeAmplitude = 0.2f;
	// Smooth lobes plus a touch of surface roughness
	OctavePerlin perlin{ 2, 0.5, 0.09 };

	const float innerR = float(radius) * (1.f - lobeAmplitude) - 1.f;
	const float outerR = float(radius) * (1.f + lobeAmplitude) + 1.f;
	const int reach = int(std::ceil(outerR));

	std::vector<ivec3> blocks;
	for (int x = -reach; x <= reach; ++x) {
		for (int y = -reach; y <= reach; ++y) {
			for (int z = -reach; z <= reach; ++z) {
				float dist3 = std::sqrt(float(x * x + y * y + z * z));
				if (dist3 <= innerR) { // deep interior: always destroyed
					blocks.push_back(center + ivec3{ x, y, z });
					continue;
				}
				if (dist3 >= outerR) // beyond the largest possible lobe: never destroyed
					continue;
				// Sample coherent noise on the block's 3D position
				dvec3 p = noiseOffset + dvec3{ x, y, z };
				double n = (perlin.getNoise({ p.x, p.y })
				          + perlin.getNoise({ p.y, p.z })
				          + perlin.getNoise({ p.z, p.x })) / 3.0;
				float effRadius = float(radius) * (1.f + lobeAmplitude * float(n));

				// Rim with ragged edge: blocks well inside are always cleared, blocks
				// straddling the boundary are cleared with falling probability.
				float edge = effRadius - dist3;
				if (edge >= 1.f || (edge > -1.f && dist(rng) < (edge + 1.f) * 0.5f))
					blocks.push_back(center + ivec3{ x, y, z });
			}
		}
	}
	return blocks;
}

void Game::fillSmoothSphere(int radius, Block block) {
	if (canReloadBlocks() && m_player.getTarget().has_value()) {
		std::vector<ivec3> blocks = smoothSphere(m_player.getTarget().value(), radius);
		for (ivec3 pos : blocks) {
			m_chunkMap.setBlock(pos, block);
		}
		reloadBlocksMeshes(blocks);
	}
}

void Game::processMouseClick(sf::Time dt, Commands& commands) {
	m_player.processMouseClick(dt, commands);
}

void Game::processMouseMove(sf::Time dt) {
	m_player.processMouseMove(dt);
}

void Game::processMouseWheel(sf::Time dt, GLfloat delta) {
	m_player.processMouseWheel(dt, delta);
}

void Game::update(sf::Time dt) {
	m_player.update(dt);
	m_defaultRenderer.getShader().use().set("view", m_player.getCamera().getViewMatrix());
	m_defaultRenderer.getShader().use().set("projection", m_player.getCamera().getProjMatrix());
	m_defaultRenderer.getShader().use().set("skyColor", p_window->getClearColor());
	m_waterRenderer.getShader().use().set("view", m_player.getCamera().getViewMatrix());
	m_waterRenderer.getShader().use().set("projection", m_player.getCamera().getProjMatrix());
	m_waterRenderer.getShader().use().set("skyColor", p_window->getClearColor());
	m_waterRenderer.getShader().use().set("cameraPosition", m_player.getCamera().getPosition());

	m_chunkMap.setCenter(Converter::globalToChunk(m_player.getPosition()));
	m_chunkMap.update();

	moveOffset = fmod(moveOffset + 0.02f * dt.asSeconds(), 1.f);
	m_waterRenderer.getShader().use().set("moveOffset", moveOffset);

	bool underwater = m_chunkMap.getBlock(Converter::globalPosToBlock(m_player.getPosition())).id == +BlockID::WATER;
	m_waterRenderer.getShader().use().set("underwater", underwater);
	m_postProcessingRenderer.getShader().use().set("underwater", underwater);
}

void Game::clearRenderTarget() {
	vec3 clearColor = p_window->getClearColor();
	glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

namespace {
	// SFML does not expose the framebuffer object backing an sf::RenderTexture,
	// but activating one binds its FBO, so we can recover its handle by reading
	// the current framebuffer binding.
	GLuint getFramebufferHandle(sf::RenderTexture& renderTexture) {
		renderTexture.setActive(true);
		GLint handle = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &handle);
		return static_cast<GLuint>(handle);
	}
}

void Game::render() {
	ivec2 size = p_window->size();

	m_waterRenderer.prepare([this]() {
		m_chunkMap.render(m_player.getCamera().getFrustum(), &m_defaultRenderer);
	}, [this]() {
		clearRenderTarget();
	}, m_defaultRenderer, m_player.getCamera(), size);

	m_postProcessingRenderer.prepare([this, size]() {
		// The refraction texture already contains the whole scene (color and depth)
		// rendered from the player's point of view, so we reuse it as the base image
		// instead of rendering the terrain a second time. We blit it into the
		// post-processing target (keeping the depth buffer so the water is correctly
		// occluded) and then only draw the water on top, which samples the refraction
		// texture in the water shader.
		GLuint refractionFbo = getFramebufferHandle(m_waterRenderer.getRefractionTexture());
		GLuint renderFbo = getFramebufferHandle(m_postProcessingRenderer.getRenderTexture());

		glBindFramebuffer(GL_READ_FRAMEBUFFER, refractionFbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderFbo);
		glBlitFramebuffer(0, 0, size.x, size.y, 0, 0, size.x, size.y,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, renderFbo);

		m_chunkMap.render(m_player.getCamera().getFrustum(), nullptr, &m_waterRenderer);

		// Drawn into the post-processing FBO which still holds the world depth.
		m_blockContourDrawer.render(m_player.getTarget(),
			m_player.getCamera().getViewMatrix(), m_player.getCamera().getProjMatrix());
	}, [this]() {
		clearRenderTarget();
	});

	p_window->setActive(true);
	clearRenderTarget();
	m_postProcessingRenderer.render();

	m_pickedBlockDrawer.render(m_player.getPickedBlock(), p_window, m_defaultRenderer);
}

Player& Game::getPlayer() {
	return m_player;
}

ChunkMap & Game::getChunkMap() {
	return m_chunkMap;
}

const ChunkMap& Game::getChunkMap() const {
	return m_chunkMap;
}

Window& Game::getWindow() {
	return *p_window;
}