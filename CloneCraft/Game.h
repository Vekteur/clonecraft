#pragma once

#include <memory>
#include <thread>
#include <chrono>
#include <SFML/Window.hpp>
#include <optional>
#include <memory>
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
	Game(Window* const window, sf::Context* const context);
	~Game();

	void processKeyboard(GLfloat dt);
	void processMouseMove(GLfloat dt);
	void processMouseWheel(GLfloat delta);
	void update(GLfloat dt);
	void render();
	void runChunkLoadingLoop();

	void onChangedSize(ivec2 size);

	Camera& getCamera();
	ChunkMap& getChunkMap();
	std::optional<ivec3> getTarget();

private:
	void clearRenderTarget();

	static const float TARGET_DISTANCE;

	Window* const p_window{ nullptr };
	sf::Context* const p_context{ nullptr };
	Camera m_camera;
	ChunkMap m_chunks;
	std::optional<ivec3> targetBlock;

	float moveOffset = 0;

	DefaultRenderer defaultRenderer;
	WaterRenderer waterRenderer;

	std::thread m_chunkMapThread;
	bool stopChunkMapThread{ false };
};