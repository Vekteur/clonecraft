#include "Game.h"

#include <random>

#include "Maths/Converter.h"
#include "Util/Debug.h"
#include "Util/Logger.h"
#include "Block/BlockDatas.h"

const float Game::TARGET_DISTANCE{ static_cast<float>(ChunkMap::VIEW_DISTANCE * Const::SECTION_SIDE) };

Game::Game(Window* const window, sf::Context* const context1, sf::Context* const context2)
	: m_camera{ vec3{0.0f, 80.0f, 0.0f } }, p_window{ window }, p_context1{ context1 }, p_context2{ context2 },
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
		m_chunkMap.load(m_camera.getFrustum());
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
	GLfloat dtSec = dt.asSeconds();
	if (commands.isActive(Command::FORWARD))
		m_camera.move(Camera::FORWARD, dtSec);
	if (commands.isActive(Command::BACKWARD))
		m_camera.move(Camera::BACKWARD, dtSec);
	if (commands.isActive(Command::LEFT))
		m_camera.move(Camera::LEFT, dtSec);
	if (commands.isActive(Command::RIGHT))
		m_camera.move(Camera::RIGHT, dtSec);
	if (commands.isActive(Command::UP))
		m_camera.move(Camera::UP, dtSec);
	if (commands.isActive(Command::DOWN))
		m_camera.move(Camera::DOWN, dtSec);
	if (commands.isActive(Command::EXPLODE))
		explode();
	if (commands.isActive(Command::TELEPORT))
		teleport();
}

void Game::teleport() {
	if (targetPos.has_value())
		m_camera.setPosition(targetPos.value());
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

void Game::explode() {
	if (canReloadBlocks() && targetPos.has_value()) {
		std::vector<ivec3> blocks = explosionBlocks(targetPos.value(), 15);
		for (ivec3 pos : blocks) {
			m_chunkMap.setBlock(pos, +ID::AIR); // Try STONE
		}
		reloadBlocks(blocks);
	}
}

void Game::processMouseClick(sf::Time dt, Commands& commands) {
	if (commands.isActive(Command::PICK) && targetPos.has_value()) {
		pickedBlock = m_chunkMap.getBlock(targetPos.value());
	}
	breakAccumulator += dt;
	if (commands.isActive(Command::BREAK) && breakAccumulator >= sf::seconds(0.1f) && targetPos.has_value()
		&& canReloadBlocks()) {

		m_chunkMap.setBlock(targetPos.value(), +ID::AIR);
		reloadBlocks({ targetPos.value() });
		breakAccumulator = sf::seconds(0.f);
	}
	placeAccumulator += dt;
	if (commands.isActive(Command::PLACE) && placeAccumulator >= sf::seconds(0.1f) && placePos.has_value() &&
		pickedBlock.has_value() && canReloadBlocks()) {

		m_chunkMap.setBlock(placePos.value(), pickedBlock.value());
		reloadBlocks({ placePos.value() });
		placeAccumulator = sf::seconds(0.f);
	}
}

void Game::processMouseMove(sf::Time dt) {
	sf::Vector2i mousePosTemp{ sf::Mouse::getPosition(*p_window) };
	ivec2 mousePosition{ mousePosTemp.x, mousePosTemp.y };
	ivec2 windowCenter{ p_window->getCenter() };
	ivec2 mouseOffset{ mousePosition - windowCenter };
	m_camera.processMouse(mouseOffset);
}

void Game::processMouseWheel(sf::Time dt, GLfloat delta) {
	m_camera.processMouseScroll(delta);
}

void Game::update(sf::Time dt) {
	m_camera.update({ p_window->size() });
	m_defaultRenderer.getShader().use().set("view", m_camera.getViewMatrix());
	m_defaultRenderer.getShader().use().set("projection", m_camera.getProjMatrix());
	m_defaultRenderer.getShader().use().set("skyColor", p_window->getClearColor());
	m_waterRenderer.getShader().use().set("view", m_camera.getViewMatrix());
	m_waterRenderer.getShader().use().set("projection", m_camera.getProjMatrix());
	m_waterRenderer.getShader().use().set("skyColor", p_window->getClearColor());
	m_waterRenderer.getShader().use().set("cameraPosition", m_camera.getPosition());

	m_chunkMap.setCenter(Converter::globalToChunk(m_camera.getPosition()));
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
		m_chunkMap.render(m_camera.getFrustum(), &m_defaultRenderer);
	}, [this]() {
		clearRenderTarget(); 
	}, m_defaultRenderer, m_camera, p_window->size());

	m_postProcessingRenderer.prepare([this]() {
		m_chunkMap.render(m_camera.getFrustum(), &m_defaultRenderer, &m_waterRenderer);
		
		// TODO : use the refraction texture both for rendering and in the water shaders

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

Camera& Game::getCamera() {
	return m_camera;
}

ChunkMap & Game::getChunkMap() {
	return m_chunkMap;
}

std::optional<ivec3> Game::getTarget() {
	return targetPos;
}
