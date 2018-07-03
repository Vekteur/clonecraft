#pragma once

#include "Camera.h"
#include "ResManager.h"
#include "Chunk.h"
#include "ChunkMap.h"
#include "Window.h"

#include <memory>
#include <thread>
#include <chrono>
#include <SFML/Window.hpp>

class Window;

class Game {
public:
	Game(Window* const window, sf::Context* const context);
	~Game();

	void processMouseWheel(GLfloat delta);
	void processInput(GLfloat dt);
	void update(GLfloat dt);
	void render();
	void runChunkLoadingLoop();

	Camera& getCamera();

private:
	Window* const p_window{ nullptr };
	sf::Context* const p_context{ nullptr };
	Camera m_camera;
	ChunkMap m_chunks;

	std::thread m_chunkMapThread;
	bool stopChunkMapThread{ false };
};