#pragma once

#include <memory>
#include <thread>
#include <chrono>
#include <SFML/Window.hpp>
#include <optional>
#include <memory>
#include <atomic>
#include <utility>

#include "Camera.h"
#include "ResManager.h"
#include "Chunk.h"
#include "ChunkMap.h"
#include "Window.h"
#include "LineBlockFinder.h"
#include "DefaultRenderer.h"
#include "WaterRenderer.h"

class Window;

class Game {
public:
	Game(Window* const window, sf::Context* const context1, sf::Context* const context2);
	~Game();

	void processKeyboard(GLfloat dt);
	void processMouseMove(GLfloat dt);
	void processMouseWheel(GLfloat delta);
	void update(GLfloat dt);
	void render();
	void runChunkLoadingLoop(sf::Context* const p_context);

	void onChangedSize(ivec2 size);

	Camera& getCamera();
	ChunkMap& getChunkMap();
	std::optional<ivec3> getTarget();

private:
	void clearRenderTarget();

	static const float TARGET_DISTANCE;

	Window* const p_window{ nullptr };
	sf::Context* const p_context1{ nullptr };
	sf::Context* const p_context2{ nullptr };
	Camera m_camera;
	ChunkMap m_chunkMap;
	std::optional<ivec3> targetBlock;

	float moveOffset = 0;
	sf::Time updateAccumulator = sf::seconds(0.f);
	std::atomic<bool> updated{ true };

	DefaultRenderer defaultRenderer;
	WaterRenderer waterRenderer;

	std::thread m_generatingThread;
	std::thread m_updatingThread;
	bool stopChunkMapThread{ false };
};