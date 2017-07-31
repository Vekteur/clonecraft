#pragma once

#include "Camera.h"
#include "ResManager.h"
#include "Chunk.h"

#include <memory>
#include <ChunkMap.h>
#include <thread>
#include <chrono>

class Window;

class Game
{
public:
	Game(Window* const window);
	~Game();

	void processInput(GLfloat dt);
	void update(GLfloat dt);
	void render();
	void runChunkLoadingLoop();

	Camera& getCamera();

private:
	Window* const p_window{ nullptr };
	Camera m_camera;
	ChunkMap m_chunks;

	vec2 m_lastMousePosition;

	std::thread m_chunkMapThread;
	bool stopChunkMapThread{ false };
};