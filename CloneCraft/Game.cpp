#include "Game.h"

#include "Converter.h"
#include "Debug.h"
#include "Logger.h"
#include "BlockDatas.h"

#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

const float Game::TARGET_DISTANCE{ 300.f };

Game::Game(Window* const window, sf::Context* const context)
	: m_camera{ vec3{0.0f, 80.0f, 0.0f } }, p_window{ window }, p_context{ context } {
	ResManager::setShader(ResManager::loadShaderFromFile("Resources/Shaders/cube.vs", "Resources/Shaders/cube.frag"), "cube");

	ResManager::getShader("cube").use().set("distance", ChunkMap::SIDE);

	m_chunkMapThread = std::thread{ &Game::runChunkLoadingLoop, this };

	std::string blockTexturesPath = "Resources/Textures/Blocks";
	std::vector<fs::path> paths;
	for (const fs::directory_entry& entry : fs::directory_iterator(blockTexturesPath)) {
		if (entry.path().extension().string() == ".png") {
			paths.push_back(entry.path());
		}
	}
	blockTextureArray = TextureArray{ paths, ivec2{ 16, 16 }, GL_RGBA };
	ResManager::initBlockDatas(std::vector<TextureArray>{ blockTextureArray });

	defaultRenderer = std::make_unique<DefaultRenderer>(&blockTextureArray);
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
	sf::Vector2f screenDim = p_window->getView().getSize();
	m_camera.update({ screenDim.x, screenDim.y });
	ResManager::getShader("cube").use().set("view", m_camera.getViewMatrix());
	ResManager::getShader("cube").use().set("projection", m_camera.getProjMatrix());
	ResManager::getShader("cube").use().set("skyColor", vec3{ 70.f / 255, 190.f / 255, 240.f / 255 });

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
	/*if (targetBlock.has_value()) {
		m_chunks.setBlock(targetBlock.value(), 0);
		m_chunks.getChunk(Converter::globalToChunk(targetBlock.value())).setState(Chunk::TO_LOAD_FACES);
	}*/

}

void Game::render() {
	m_chunks.render(m_camera.getFrustum(), *defaultRenderer);
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
