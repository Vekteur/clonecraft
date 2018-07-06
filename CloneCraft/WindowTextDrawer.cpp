#include "WindowTextDrawer.h"

#include <iomanip>

#include "Logger.h"

WindowTextDrawer::WindowTextDrawer(Window * window) : p_window{ window } {
	if (!m_font.loadFromFile("Resources/Fonts/arial.ttf")) {
		LOG(Level::ERROR) << "Failed to load font" << std::endl;
	}
}

void WindowTextDrawer::draw(int fps, Game& game) {
	drawFPS(fps);
	drawPosition(game.getCamera().getPosition());
	drawDirection(game.getCamera().getYaw(), game.getCamera().getPitch());
	drawChunksInfos(game.getChunkMap().chunksAtLeastInState(Chunk::TO_LOAD_FACES),
		game.getChunkMap().chunksAtLeastInState(Chunk::TO_LOAD_VAOS));
	drawBlockNumber(game.getChunkMap().chunksAtLeastInState(Chunk::TO_LOAD_FACES) *
		Const::CHUNK_SIDE * Const::CHUNK_SIDE * Const::CHUNK_HEIGHT);
}

void WindowTextDrawer::drawFPS(int fps) {
	std::ostringstream oss;
	oss << "FPS : " << std::setw(4) << fps;
	drawAtLine(oss.str(), 0);
}

void WindowTextDrawer::drawPosition(vec3 pos) {
	std::ostringstream oss;
	oss << "XYZ : " << std::fixed << std::setw(10) << std::setprecision(3) << pos.x << ' ' << 
		std::fixed << std::setw(10) << pos.y << ' ' << std::fixed << std::setw(10) << pos.z;
	drawAtLine(oss.str(), 1);
}

void WindowTextDrawer::drawDirection(float pitch, float yaw) {
	std::ostringstream oss;
	oss << "Pitch / Yaw : " << std::fixed << std::setw(8) << std::setprecision(3) << pitch << ' ' << 
		std::fixed << std::setw(8) << std::setprecision(3) << yaw;
	drawAtLine(oss.str(), 2);
}

void WindowTextDrawer::drawChunksInfos(int blockChunks, int faceChunks) {
	std::ostringstream oss;
	oss << "Block Chunks : " << std::setw(6) << blockChunks << "     Face Chunks : " << std::setw(6) << faceChunks;
	drawAtLine(oss.str(), 3);
}

void WindowTextDrawer::drawBlockNumber(int blockNumber) {
	std::ostringstream oss;
	oss << "Blocks : " << std::setw(12) << blockNumber;
	drawAtLine(oss.str(), 4);
}

void WindowTextDrawer::drawAtLine(std::string message, int line) {
	sf::Text text;
	text.setFont(m_font);
	text.setString(message);
	text.setCharacterSize(textSize * p_window->getSize().y);
	text.setPosition({ paddingLeft * p_window->getSize().x, 
		p_window->getSize().y * (paddingTop + line * textSpacing) });
	p_window->draw(text);
}
