#include "Game.h"

#include "Converter.h"
#include "Debug.h"

Game::Game(Window* const window, sf::Context* const context)
	: m_camera{ vec3{0.0f, 80.0f, 0.0f } }, p_window{ window }, p_context{ context } {
	ResManager::loadShader("Resources/Shaders/cube.vs", "Resources/Shaders/cube.frag", nullptr, "cube");
	ResManager::loadTexture("Resources/Textures/stone.png", GL_FALSE, "stone");

	ResManager::getShader("cube").use().setInt("distance", ChunkMap::SIDE);

	if (glGetError())
		std::cin.get();
	m_chunkMapThread = std::thread{ &Game::runChunkLoadingLoop, this };
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

void Game::processKeyboard(GLfloat dt) {
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
		m_camera.move(Camera::FORWARD, dt);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		m_camera.move(Camera::BACKWARD, dt);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
		m_camera.move(Camera::LEFT, dt);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		m_camera.move(Camera::RIGHT, dt);
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
	ResManager::getShader("cube").use().setMat4("view", m_camera.getViewMatrix());
	ResManager::getShader("cube").use().setMat4("projection", m_camera.getProjectionMatrix());
	ResManager::getShader("cube").use().setVec3("skyColor", { 70.f / 255, 190.f / 255, 240.f / 255 });

	ivec2 newCenter = Converter::globalToChunk(m_camera.getPosition());
	if (m_chunks.getCenter() != newCenter)
		m_chunks.setCenter(newCenter);

	m_chunks.update();
}

void Game::render() {
	m_chunks.render();
}

Camera& Game::getCamera() {
	return m_camera;
}

ChunkMap & Game::getChunkMap() {
	return m_chunks;
}
