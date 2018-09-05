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
#include "Commands.h"

class Window;

class Game {
public:
	Game(Window* const window, sf::Context* const context1, sf::Context* const context2);
	~Game();

	void processKeyboard(GLfloat dt, Commands& commands);
	void processMouseClick(GLfloat dt, Commands& commands);
	void processMouseMove(GLfloat dt);
	void processMouseWheel(GLfloat dt);
	void update(GLfloat dt);
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

	std::thread m_generatingThread;
	std::thread m_updatingThread;
	bool m_stopGeneratingThread{ false };
};