#pragma once

#include <SFML/Window.hpp>

#include <memory>
#include <thread>
#include <chrono>
#include <optional>
#include <memory>
#include <atomic>
#include <utility>
#include <vector>

#include "Camera.h"
#include "ResManager.h"
#include "Chunk.h"
#include "ChunkMap.h"
#include "Window.h"
#include "LineBlockFinder.h"
#include "DefaultRenderer.h"
#include "WaterRenderer.h"
#include "PostProcessingRenderer.h"
#include "Commands.h"

class Window;

class Game {
public:
	Game(Window* const window, sf::Context* const context1, sf::Context* const context2);
	~Game();

	void processKeyboard(sf::Time dt, Commands& commands);
	void processMouseClick(sf::Time dt, Commands& commands);
	void processMouseMove(sf::Time dt);
	void processMouseWheel(sf::Time dt, GLfloat delta);
	void update(sf::Time dt);
	void render();
	void runChunkLoadingLoop(sf::Context* const p_context);

	void onChangedSize(ivec2 size);

	Camera& getCamera();
	ChunkMap& getChunkMap();
	std::optional<ivec3> getTarget();

private:
	bool canReloadBlocks();
	void reloadBlocks(const std::vector<ivec3>& blocks);
	void clearRenderTarget();

	std::vector<ivec3> explosionBlocks(ivec3 center, int radius);
	void explode();

	static const float TARGET_DISTANCE;

	Window* const p_window{ nullptr };
	sf::Context* const p_context1{ nullptr };
	sf::Context* const p_context2{ nullptr };
	Camera m_camera;
	ChunkMap m_chunkMap;
	std::optional<ivec3> targetPos;
	std::optional<ivec3> placePos;
	std::optional<Block> pickedBlock;

	float moveOffset = 0;
	std::atomic<bool> updatingThreadFinished{ true };

	sf::Time breakAccumulator = sf::seconds(0.f);
	sf::Time placeAccumulator = sf::seconds(0.f);

	DefaultRenderer m_defaultRenderer;
	WaterRenderer m_waterRenderer;
	PostProcessingRenderer m_postProcessingRenderer;

	std::thread m_generatingThread;
	std::thread m_updatingThread;
	bool m_stopGeneratingThread{ false };
};