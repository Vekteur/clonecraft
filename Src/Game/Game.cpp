#include "Game.h"

#include <random>

#include "Maths/Converter.h"
#include "Util/DebugGL.h"
#include "Util/Logger.h"
#include "Block/BlockDatas.h"

Game::Game(Window* const window, sf::Context* const context1, sf::Context* const context2)
	: m_player{ this }, p_window{ window }, p_context1{ context1 }, p_context2{ context2 },
	m_waterRenderer{ { p_window->size() } }, m_postProcessingRenderer{ { p_window->size() } } {

	ResManager::initBlockDatas(std::vector<TextureArray*>{ &m_defaultRenderer.getTextureArray() });
	m_generatingThread = std::thread{ &Game::runChunkLoadingLoop, this, p_context1 };
}

Game::~Game() {
	m_stopGeneratingThread = true;
	m_chunkMap.stop();
	m_generatingThread.join();
	if (m_updatingThread.joinable())
		m_updatingThread.join();
}

void Game::runChunkLoadingLoop(sf::Context* const p_context) {
	p_context->setActive(true);

	while (!m_stopGeneratingThread) {
		m_chunkMap.load(m_player.getCamera().getFrustum());
		m_chunkMap.unloadFarChunks();
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

void Game::reloadBlocks(const std::vector<ivec3>& blocks) {
	if (updatingThreadFinished) {
		updatingThreadFinished = false;
		if (m_updatingThread.joinable())
			m_updatingThread.join();
		m_updatingThread = std::thread{ [this, blocks]() {
			p_context2->setActive(true);
			m_chunkMap.reloadBlocks(blocks);
			p_context2->setActive(false);
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
	if (commands.isActive(Command::EXPLOSION))
		explode(15);
	if (commands.isActive(Command::HUGE_EXPLOSION))
		explode(100);
	if (commands.isActive(Command::TELEPORT))
		m_player.teleport();
	if (commands.isActive(Command::NEXT_GAMEMODE))
		m_player.nextGameMode();
}

std::vector<ivec3> Game::explosionBlocks(ivec3 center, int radius) {
	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<int> dist(0, radius);
	std::vector<ivec3> blocks;
	for (int x = -radius; x <= radius; ++x) {
		for (int y = -radius; y <= radius; ++y) {
			for (int z = -radius; z <= radius; ++z) {
				float randomRadius = float(radius) - float(dist(rng)) / 4.f;
				if (x * x + y * y + z * z <= randomRadius * randomRadius)
					blocks.push_back(center + ivec3{ x, y, z });
			}
		}
	}
	return blocks;
}

void Game::explode(int radius) {
	if (canReloadBlocks() && m_player.getTarget().has_value()) {
		std::vector<ivec3> blocks = explosionBlocks(m_player.getTarget().value(), radius);
		for (ivec3 pos : blocks) {
			m_chunkMap.setBlock(pos, +BlockID::AIR); // Try STONE
		}
		reloadBlocks(blocks);
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
}

void Game::clearRenderTarget() {
	vec3 clearColor = p_window->getClearColor();
	glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Game::render() {
	m_waterRenderer.prepare([this]() {
		m_chunkMap.render(m_player.getCamera().getFrustum(), &m_defaultRenderer);
	}, [this]() {
		clearRenderTarget(); 
	}, m_defaultRenderer, m_player.getCamera(), p_window->size());

	m_postProcessingRenderer.prepare([this]() {
		m_chunkMap.render(m_player.getCamera().getFrustum(), &m_defaultRenderer, &m_waterRenderer);
		
		// TODO: use the refraction texture both for rendering and in the water shaders
		// Still needs some work

		/*glBindFramebuffer(GL_READ_FRAMEBUFFER,
			m_waterRenderer.getRefractionTexture().getTexture().getNativeHandle());
		Debug::glCheckError();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
			m_postProcessingRenderer.getRenderTexture().getTexture().getNativeHandle());
		Debug::glCheckError();
		glBlitFramebuffer(0, 0, p_window->size().x, p_window->size().y, 0, 0, p_window->size().x, p_window->size().y,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		Debug::glCheckError();
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_chunkMap.render(m_camera.getFrustum(), nullptr, &m_waterRenderer);*/
	}, [this]() {
		clearRenderTarget();
	});
	
	p_window->setActive(true);
	clearRenderTarget();
	m_postProcessingRenderer.render();
}

Player& Game::getPlayer() {
	return m_player;
}

ChunkMap & Game::getChunkMap() {
	return m_chunkMap;
}

Window& Game::getWindow() {
	return *p_window;
}