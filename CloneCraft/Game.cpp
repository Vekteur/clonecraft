#include "Game.h"

#include "Converter.h"
#include "Debug.h"
#include "Logger.h"
#include "BlockDatas.h"

const float Game::TARGET_DISTANCE{ 100.f };

Game::Game(Window* const window, sf::Context* const context1, sf::Context* const context2)
	: m_camera{ vec3{0.0f, 80.0f, 0.0f } }, p_window{ window }, p_context1{ context1 }, p_context2{ context2 },
	m_waterRenderer{ { p_window->size() } } {

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
		m_chunkMap.load();
		m_chunkMap.unloadFarChunks();
	}
}

void Game::onChangedSize(ivec2 size) {
	glViewport(0, 0, size.x, size.y);
	p_window->setView(sf::View(sf::FloatRect(0, 0, size.x, size.y)));
	m_waterRenderer.onChangedSize(p_window->size());
}

void Game::processKeyboard(GLfloat dt, Commands& commands) {
	if (commands.isActive(Command::FORWARD))
		m_camera.move(Camera::FORWARD, dt);
	if (commands.isActive(Command::BACKWARD))
		m_camera.move(Camera::BACKWARD, dt);
	if (commands.isActive(Command::LEFT))
		m_camera.move(Camera::LEFT, dt);
	if (commands.isActive(Command::RIGHT))
		m_camera.move(Camera::RIGHT, dt);
	if (commands.isActive(Command::UP))
		m_camera.move(Camera::UP, dt);
	if (commands.isActive(Command::DOWN))
		m_camera.move(Camera::DOWN, dt);
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

void Game::processMouseClick(GLfloat dt, Commands& commands) {
	if (commands.isActive(Command::PICK) && targetPos.has_value()) {
		pickedBlock = m_chunkMap.getBlock(targetPos.value());
	}
	breakAccumulator += sf::seconds(dt);
	if (commands.isActive(Command::BREAK) && breakAccumulator >= sf::seconds(0.1f) && targetPos.has_value()
		&& canReloadBlocks()) {

		m_chunkMap.setBlock(targetPos.value(), +ID::AIR);
		reloadBlocks({ targetPos.value() });
		breakAccumulator = sf::seconds(0.f);
	}
	placeAccumulator += sf::seconds(dt);
	if (commands.isActive(Command::PLACE) && placeAccumulator >= sf::seconds(0.1f) && placePos.has_value() &&
		pickedBlock.has_value() && canReloadBlocks()) {

		m_chunkMap.setBlock(placePos.value(), pickedBlock.value());
		reloadBlocks({ placePos.value() });
		placeAccumulator = sf::seconds(0.f);
	}
}

void Game::processMouseMove(GLfloat dt) {
	sf::Vector2i mousePosTemp{ sf::Mouse::getPosition(*p_window) };
	ivec2 mousePosition{ mousePosTemp.x, mousePosTemp.y };
	ivec2 windowCenter{ p_window->getCenter() };
	ivec2 mouseOffset{ mousePosition - windowCenter };
	m_camera.processMouse(mouseOffset);
}

void Game::processMouseWheel(GLfloat dt) {
	m_camera.processMouseScroll(dt);
}

void Game::update(GLfloat dt) {
	m_camera.update({ p_window->size() });
	m_defaultRenderer.getShader().use().set("view", m_camera.getViewMatrix());
	m_defaultRenderer.getShader().use().set("projection", m_camera.getProjMatrix());
	m_defaultRenderer.getShader().use().set("skyColor", p_window->getClearColor());
	m_waterRenderer.getShader().use().set("view", m_camera.getViewMatrix());
	m_waterRenderer.getShader().use().set("projection", m_camera.getProjMatrix());
	m_waterRenderer.getShader().use().set("skyColor", p_window->getClearColor());
	m_waterRenderer.getShader().use().set("cameraPosition", m_camera.getPosition());

	ivec2 newCenter = Converter::globalToChunk(m_camera.getPosition());
	if (m_chunkMap.getCenter() != newCenter)
		m_chunkMap.setCenter(newCenter);

	m_chunkMap.update();

	LineBlockFinder lineBlockFinder{ m_camera.getPosition(), m_camera.getFront() };
	placePos = std::nullopt;
	targetPos = std::nullopt;
	while (lineBlockFinder.getDistance() <= TARGET_DISTANCE) {
		ivec3 iterPos = lineBlockFinder.next();
		Block block = m_chunkMap.getBlock(iterPos);
		if (ResManager::blockDatas().get(block.id).getCategory() != BlockData::AIR &&
			ResManager::blockDatas().get(block.id).getCategory() != BlockData::WATER) {
			targetPos = iterPos;
			break;
		}
		placePos = iterPos;
	}
	if (targetPos == std::nullopt)
		placePos = std::nullopt;

	moveOffset = fmod(moveOffset + 0.02f * dt, 1.f);
	m_waterRenderer.getShader().use().set("moveOffset", moveOffset);
}

void Game::clearRenderTarget() {
	vec3 clearColor = p_window->getClearColor();
	glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Game::render() {
	m_waterRenderer.prepare([&]() {
		m_chunkMap.render(m_camera.getFrustum(), &m_defaultRenderer);
	}, [&]() { 
		clearRenderTarget(); 
	}, m_defaultRenderer, m_camera, p_window->size());

	p_window->setActive(true);
	clearRenderTarget();
	m_chunkMap.render(m_camera.getFrustum(), &m_defaultRenderer, &m_waterRenderer);
}

Camera& Game::getCamera() {
	return m_camera;
}

ChunkMap & Game::getChunkMap() {
	return m_chunkMap;
}

std::optional<ivec3> Game::getTarget() {
	return targetPos;
}
