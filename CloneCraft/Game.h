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

	void processKeyboard(GLfloat dt);
	void processMouseMove(GLfloat dt);
	void processMouseWheel(GLfloat delta);
	void update(GLfloat dt);
	void render();
	void runChunkLoadingLoop();

	Camera& getCamera();
	ChunkMap& getChunkMap();

private:
	Window* const p_window{ nullptr };
	sf::Context* const p_context{ nullptr };
	Camera m_camera;
	ChunkMap m_chunks;

	std::thread m_chunkMapThread;
	bool stopChunkMapThread{ false };
};