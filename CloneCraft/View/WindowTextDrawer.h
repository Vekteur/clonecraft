#pragma once

#include <SFML/Graphics.hpp>

#include "Window.h"
#include "GlmCommon.h"
#include "Game.h"

class WindowTextDrawer
{
public:
	WindowTextDrawer(Window* window);
	void drawAll(int fps, Game& game);

private:
	const float paddingLeft = 0.005f;
	const float paddingTop = 0.005f;
	const float textSize = 0.025f;
	const float textSpacing = 0.03f;

	Window * p_window;
	sf::Font m_font;
	int line = 0;

	void drawFPS(int fps);
	void drawGlobalPosition(vec3 pos);
	void drawLocalPosition(vec3 pos);
	void drawSectionPosition(ivec3 pos);
	void drawTarget(std::optional<ivec3> pos);
	void drawDirection(float pitch, float yaw);
	void drawRenderedChunks(int renderedChunks);
	void drawBlockChunks(int blockChunks);
	void drawFaceChunks(int faceChunks);
	void drawBlockNumber(int blockNumber);

	void draw(std::string message);
};