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
#include "Renderer/DefaultRenderer.h"
#include "Renderer/WaterRenderer.h"
#include "Renderer/PostProcessingRenderer.h"
#include "Commands/Commands.h"
#include "Game/Player.h"

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
	bool canReloadBlocks();
	void reloadBlocks(const std::vector<ivec3>& blocks);

	Player& getPlayer();
	ChunkMap& getChunkMap();
	Window& getWindow();

private:
	void clearRenderTarget();

	std::vector<ivec3> explosionBlocks(ivec3 center, int radius);
	void explode(int radius);

	Player m_player;

	Window* const p_window{ nullptr };
	sf::Context* const p_context1{ nullptr };
	sf::Context* const p_context2{ nullptr };
	ChunkMap m_chunkMap;

	float moveOffset = 0;
	std::atomic<bool> updatingThreadFinished{ true };

	DefaultRenderer m_defaultRenderer;
	WaterRenderer m_waterRenderer;
	PostProcessingRenderer m_postProcessingRenderer;

	std::thread m_generatingThread;
	std::thread m_updatingThread;
	bool m_stopGeneratingThread{ false };
};