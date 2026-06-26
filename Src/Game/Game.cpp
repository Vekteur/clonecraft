#include "Game.h"

#include "Game/BulkEdit.h"
#include "Maths/Converter.h"
#include "Util/DebugGL.h"
#include "Util/Logger.h"
#include "World/WorldConstants.h"

#include <cmath>

Game::Game(Window* const window)
	: m_player{ this }, p_window{ window },
	m_defaultRenderer{ m_player.getCamera(), m_dayCycle },
	m_waterRenderer{ p_window->size(), m_player.getCamera(), m_dayCycle },
	m_lavaRenderer{ m_player.getCamera(), m_dayCycle },
	m_postProcessingRenderer{ p_window->size(), m_dayCycle } {

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

void Game::submitSphereEdit(int radius, Block block, bool explosive) {
	if (!m_player.getTarget().has_value())
		return;
	ivec3 center = m_player.getTarget().value();
	// Dropped if a bulk edit is already in flight (at most one at a time).
	bool accepted = m_chunkMap.submitBulkEdit([center, radius, block]() {
		return BulkEdit::smoothSphere(center, radius, block);
	});
	// Kick off the visual on the main thread the moment the edit is accepted, so the fireball
	// plays over the block center while the worker thread computes and applies the changes.
	if (accepted && explosive)
		m_explosionDrawer.spawn(vec3(center) + vec3(0.5f), float(radius));
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
		submitSphereEdit(15, +BlockID::AIR, true);
	if (commands.isActive(Command::HUGE_EXPLOSION))
		submitSphereEdit(100, +BlockID::AIR, true);
	if (commands.isActive(Command::BRUSH))
		if (m_player.getPickedBlock().has_value())
			submitSphereEdit(15, m_player.getPickedBlock().value());
	if (commands.isActive(Command::HUGE_BRUSH))
		if (m_player.getPickedBlock().has_value())
			submitSphereEdit(100, m_player.getPickedBlock().value());
	if (commands.isActive(Command::PLACE_BELOW))
		m_player.placeBlockBelow();
	if (commands.isActive(Command::TELEPORT))
		m_player.teleport();
	if (commands.isActive(Command::NEXT_GAMEMODE))
		m_player.nextGameMode();
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
	m_dayCycle.update(dt);

	m_chunkMap.setCenter(Converter::globalToChunk(m_player.getPosition()));
	m_chunkMap.update();

	BlockID eyeBlock = m_chunkMap.getBlock(Converter::globalPosToBlock(m_player.getPosition())).id;
	m_waterRenderer.setUnderwater(eyeBlock == +BlockID::WATER);
	m_postProcessingRenderer.setUnderwater(eyeBlock == +BlockID::WATER);
	m_postProcessingRenderer.setInLava(eyeBlock == +BlockID::LAVA);

	m_defaultRenderer.update(dt);
	m_waterRenderer.update(dt);
	m_lavaRenderer.update(dt);
	m_postProcessingRenderer.update(dt);

	m_explosionDrawer.update(dt.asSeconds());
}

void Game::clearRenderTarget() {
	// Dimmed by the day/night cycle (set in update) so the sky and fog darken with the terrain.
	vec3 skyColor = m_dayCycle.getSkyColor();
	glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
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

	m_waterRenderer.prepare([this](bool refractionPass) {
		// In the reflection pass we clip at the sea level, the same plane the terrain uses, so things
		// below the surface are not mirrored as phantoms above it.
		vec4 clipPlane = refractionPass ? vec4(0.f, -1.f, 0.f, 10000.f)
		                                : vec4(0.f, 1.f, 0.f, -float(Const::SEA_LEVEL));
		// Lava is opaque, so it rides along in the terrain pass and ends up in the scene textures the
		// water shader composites from.
		m_lavaRenderer.getShader().use().set("clipPlane", clipPlane);
		m_chunkMap.render(m_player.getCamera().getFrustum(), { &m_defaultRenderer, &m_lavaRenderer });
		// Drawn into the scene textures rather than over the final image, so the water shader (which
		// composites the scene from the refraction texture) shows the fireball through and underneath
		// the water.
		m_explosionDrawer.render(m_player.getCamera().getViewMatrix(),
			m_player.getCamera().getProjMatrix(), clipPlane, m_dayCycle.getSkyColor());
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

		m_chunkMap.render(m_player.getCamera().getFrustum(), { &m_waterRenderer });

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

float Game::getTimeOfDay() const {
	return m_dayCycle.getTimeOfDay();
}