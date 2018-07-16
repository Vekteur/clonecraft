#include "Game.h"

#include "Converter.h"
#include "Debug.h"
#include "Logger.h"
#include "BlockDatas.h"

#include <vector>

const float Game::TARGET_DISTANCE{ 300.f };

Game::Game(Window* const window, sf::Context* const context)
	: m_camera{ vec3{0.0f, 80.0f, 0.0f } }, p_window{ window }, p_context{ context } {
	
	ResManager::getShader("cube").use().set("distance", ChunkMap::SIDE);
	ResManager::getShader("water").use().set("distance", ChunkMap::SIDE);

	m_chunkMapThread = std::thread{ &Game::runChunkLoadingLoop, this };

	ResManager::initBlockDatas(std::vector<TextureArray*>{ &defaultRenderer.getTextureArray() });

	sf::ContextSettings settings;
	settings.majorVersion = 4;
	settings.minorVersion = 3;
	settings.depthBits = 24;
	settings.stencilBits = 8;

	if (!reflectionTexture.create(p_window->getSize().x / 2, p_window->getSize().y / 2, settings)) {
		LOG(Level::ERROR) << "Could not create reflection texture" << std::endl;
	}
	if (!refractionTexture.create(p_window->getSize().x / 2, p_window->getSize().y / 2, settings)) {
		LOG(Level::ERROR) << "Could not create refraction texture" << std::endl;
	}
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
	ResManager::getShader("cube").use().set("skyColor", p_window->getClearColor());
	ResManager::getShader("water").use().set("view", m_camera.getViewMatrix());
	ResManager::getShader("water").use().set("projection", m_camera.getProjMatrix());
	ResManager::getShader("water").use().set("skyColor", p_window->getClearColor());

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

void Game::reflectCamera() {
	vec3 pos = m_camera.getPosition();
	m_camera.setPosition({ pos.x, 2 * Const::SEA_LEVEL - pos.y , pos.z });
	m_camera.invertPitch();
	sf::Vector2f screenDim = p_window->getView().getSize();
	m_camera.update({ screenDim.x, screenDim.y });

	ResManager::getShader("cube").use().set("view", m_camera.getViewMatrix());
	ResManager::getShader("cube").use().set("projection", m_camera.getProjMatrix());
	ResManager::getShader("water").use().set("view", m_camera.getViewMatrix());
	ResManager::getShader("water").use().set("projection", m_camera.getProjMatrix());
}

void Game::clearRenderTarget() {
	vec3 clearColor = p_window->getClearColor();
	glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Game::render() {

	reflectCamera();

	ResManager::getShader("cube").use().set("clipPlane", vec4(0, 1, 0, -Const::SEA_LEVEL));
	reflectionTexture.setActive(true);
	clearRenderTarget();
	m_chunks.render(m_camera.getFrustum(), defaultRenderer);
	reflectionTexture.display();

	reflectCamera();

	ResManager::getShader("cube").use().set("clipPlane", vec4(0, -1, 0, Const::SEA_LEVEL));
	refractionTexture.setActive(true);
	clearRenderTarget();
	m_chunks.render(m_camera.getFrustum(), defaultRenderer);
	refractionTexture.display();

	ResManager::getShader("cube").use().set("clipPlane", vec4(0, -1, 0, 10000));
	p_window->setActive(true);
	clearRenderTarget();
	m_chunks.render(m_camera.getFrustum(), defaultRenderer, &waterRenderer);
	p_window->pushGLStates();
	sf::Sprite reflectionSprite(reflectionTexture.getTexture());
	sf::Sprite refractionSprite(refractionTexture.getTexture());
	refractionSprite.setPosition(p_window->getSize().x / 2, 0);
	p_window->draw(reflectionSprite);
	p_window->draw(refractionSprite);
	p_window->popGLStates();
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
