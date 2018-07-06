#pragma once

#include <SFML/Graphics.hpp>

#include "Window.h"
#include "GlmCommon.h"
#include "Game.h"

class WindowTextDrawer
{
public:
	WindowTextDrawer(Window* window);
	void draw(int fps, Game& game);

private:
	const float paddingLeft = 0.005;
	const float paddingTop = 0.005;
	const float textSize = 0.025;
	const float textSpacing = 0.03;

	Window * p_window;
	sf::Font m_font;

	void drawFPS(int fps);
	void drawPosition(vec3 pos);
	void drawDirection(float pitch, float yaw);
	void drawChunksInfos(int blockChunks, int faceChunks);
	void drawBlockNumber(int blockNumber);
	void drawAtLine(std::string message, int line);
};