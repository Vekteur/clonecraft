#include "Game.h"

#include "Converter.h"
#include "Debug.h"
#include "Logger.h"
#include "BlockDatas.h"

#include <vector>

const float Game::TARGET_DISTANCE{ 300.f };

Game::Game(Window* const window, sf::Context* const context)
	: m_camera{ vec3{0.0f, 80.0f, 0.0f } }, p_window{ window }, p_context{ context }, 
	waterRenderer{ { p_window->size() } } {

	m_chunkMapThread = std::thread{ &Game::runChunkLoadingLoop, this };

	ResManager::initBlockDatas(std::vector<TextureArray*>{ &defaultRenderer.getTextureArray() });
}

Game::~Game() {
	stopChunkMapThread = true;
	m_chunks.stop();
	m_chunkMapThread.join();
}

void Game::runChunkLoadingLoop() {
	p_context->setActive(true);

	while (!stopChunkMapThread) {
		m_chunks.load();
		m_chunks.unloadFarChunks();
	}
}

void Game::onChangedSize(ivec2 size) {
	glViewport(0, 0, size.x, size.y);
	p_window->setView(sf::View(sf::FloatRect(0, 0, size.x, size.y)));
	waterRenderer.onChangedSize(p_window->size());
}

void Game::processKeyboard(GLfloat dt) {
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
		m_camera.move(Camera::FORWARD, dt);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		m_camera.move(Camera::BACKWARD, dt);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
		m_camera.move(Camera::LEFT, dt);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		m_camera.move(Camera::RIGHT, dt);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		m_camera.move(Camera::UP, dt);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
		m_camera.move(Camera::DOWN, dt);
}

void Game::processMouseMove(GLfloat dt) {
	sf::Vector2i mousePosTemp{ sf::Mouse::getPosition(*p_window) };
	ivec2 mousePosition{ mousePosTemp.x, mousePosTemp.y };
	ivec2 windowCenter{ p_window->getCenter() };
	ivec2 mouseOffset{ mousePosition - windowCenter };
	m_camera.processMouse(mouseOffset);
}

void Game::processMouseWheel(GLfloat delta) {
	m_camera.processMouseScroll(delta);
}

void Game::update(GLfloat dt) {
	m_camera.update({ p_window->size() });
	defaultRenderer.getShader().use().set("view", m_camera.getViewMatrix());
	defaultRenderer.getShader().use().set("projection", m_camera.getProjMatrix());
	defaultRenderer.getShader().use().set("skyColor", p_window->getClearColor());
	waterRenderer.getShader().use().set("view", m_camera.getViewMatrix());
	waterRenderer.getShader().use().set("projection", m_camera.getProjMatrix());
	waterRenderer.getShader().use().set("skyColor", p_window->getClearColor());
	waterRenderer.getShader().use().set("cameraPosition", m_camera.getPosition());

	ivec2 newCenter = Converter::globalToChunk(m_camera.getPosition());
	if (m_chunks.getCenter() != newCenter)
		m_chunks.setCenter(newCenter);

	m_chunks.update();

	LineBlockFinder lineBlockFinder{ m_camera.getPosition(), m_camera.getFront() };
	targetBlock = std::nullopt;
	while (lineBlockFinder.getDistance() <= TARGET_DISTANCE) {
		ivec3 iterBlock = lineBlockFinder.next();
		if (m_chunks.getBlock(iterBlock).id != +ID::AIR) {
			targetBlock = iterBlock;
			break;
		}
	}
	moveOffset = fmod(moveOffset + 0.02f * dt, 1.f);
	waterRenderer.getShader().use().set("moveOffset", moveOffset);

	/*if (targetBlock.has_value()) {
		m_chunks.setBlock(targetBlock.value(), 0);
		m_chunks.getChunk(Converter::globalToChunk(targetBlock.value())).setState(Chunk::TO_LOAD_FACES);
	}*/
}

void Game::clearRenderTarget() {
	vec3 clearColor = p_window->getClearColor();
	glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Game::render() {

	waterRenderer.prepare([&]() {
		m_chunks.render(m_camera.getFrustum(), &defaultRenderer);
	}, [&]() { 
		clearRenderTarget(); 
	}, defaultRenderer, m_camera, p_window->size());

	p_window->setActive(true);
	clearRenderTarget();
	m_chunks.render(m_camera.getFrustum(), &defaultRenderer, &waterRenderer);
}

Camera& Game::getCamera() {
	return m_camera;
}

ChunkMap & Game::getChunkMap() {
	return m_chunks;
}

std::optional<ivec3> Game::getTarget() {
	return targetBlock;
}
