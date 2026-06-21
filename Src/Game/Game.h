#pragma once

#include <SFML/Window.hpp>

#include <memory>
#include <thread>
#include <chrono>
#include <memory>
#include <atomic>
#include <utility>
#include <vector>

#include "ResManager/ResManager.h"
#include "World/Chunk.h"
#include "World/ChunkMap.h"
#include "View/Window.h"
#include "View/PickedBlockDrawer.h"
#include "Renderer/DefaultRenderer.h"
#include "Renderer/WaterRenderer.h"
#include "Renderer/PostProcessingRenderer.h"
#include "Commands/Commands.h"
#include "Game/Player.h"

class Window;

class Game {
public:
	Game(Window* const window);
	~Game();

	void processKeyboard(sf::Time dt, Commands& commands);
	void processMouseClick(sf::Time dt, Commands& commands);
	void processMouseMove(sf::Time dt);
	void processMouseWheel(sf::Time dt, GLfloat delta);
	void update(sf::Time dt);
	void render();
	void runOrchestratorLoop();
	void runWorkerLoop();
	void onChangedSize(ivec2 size);
	bool canReloadBlocks();
	void reloadBlocksMeshes(const std::vector<ivec3>& blocks);

	Player& getPlayer();
	ChunkMap& getChunkMap();
	Window& getWindow();

private:
	void clearRenderTarget();
	void buildHeldBlockMesh(Block block);

	std::vector<ivec3> smoothSphere(ivec3 center, int radius);
	void fillSmoothSphere(int radius, Block block);

	Player m_player;

	Window* const p_window{ nullptr };
	ChunkMap m_chunkMap;

	float moveOffset = 0;
	std::atomic<bool> updatingThreadFinished{ true };

	DefaultRenderer m_defaultRenderer;
	WaterRenderer m_waterRenderer;
	PostProcessingRenderer m_postProcessingRenderer;

	PickedBlockDrawer m_pickedBlockDrawer;

	std::thread m_orchestratorThread;
	std::vector<std::thread> m_workerThreads;
	std::thread m_updatingThread;
	std::atomic<bool> m_stopGeneratingThread{ false };
};