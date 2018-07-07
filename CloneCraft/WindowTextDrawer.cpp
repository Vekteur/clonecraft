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
	vec3 pos = game.getCamera().getPosition();
	drawGlobalPosition(pos);
	drawLocalPosition(Converter::globalToInnerSection(pos));
	drawSectionPosition(Converter::globalToSection(pos));
	drawTarget(game.getTarget());
	drawDirection(game.getCamera().getYaw(), game.getCamera().getPitch());
	drawBlockChunks(game.getChunkMap().chunksAtLeastInState(Chunk::TO_LOAD_FACES));
	drawFaceChunks(game.getChunkMap().chunksAtLeastInState(Chunk::TO_LOAD_VAOS));
	drawBlockNumber(game.getChunkMap().chunksAtLeastInState(Chunk::TO_LOAD_FACES) *
		Const::CHUNK_SIDE * Const::CHUNK_SIDE * Const::CHUNK_HEIGHT);
}

void WindowTextDrawer::drawFPS(int fps) {
	std::ostringstream oss;
	oss << "FPS : " << std::setw(4) << fps;
	drawAtLine(oss.str(), 0);
}

void WindowTextDrawer::drawGlobalPosition(vec3 pos) {
	std::ostringstream oss;
	oss << "Global :  " << std::setprecision(2) << std::fixed << std::setw(10) << std::right << pos.x << ' ' <<
		std::fixed << std::setw(10) << std::right << pos.y << ' ' << std::fixed << std::setw(10) << std::right << pos.z;
	drawAtLine(oss.str(), 1);
}

void WindowTextDrawer::drawLocalPosition(vec3 pos) {
	std::ostringstream oss;
	oss << "Local :   " << std::setprecision(4) << std::fixed << std::setw(8) << pos.x << ' ' <<
		std::fixed << std::setw(8) << pos.y << ' ' << std::fixed << std::setw(8) << pos.z;
	drawAtLine(oss.str(), 2);
}

void WindowTextDrawer::drawSectionPosition(ivec3 pos) {
	std::ostringstream oss;
	oss << "Section : " << std::setw(12) << pos.x << ' ' <<
		std::setw(12) << pos.y << ' ' << std::setw(12) << pos.z;
	drawAtLine(oss.str(), 3);
}

void WindowTextDrawer::drawTarget(std::optional<ivec3> opt_pos) {
	std::ostringstream oss;
	oss << "Target : ";
	if (opt_pos.has_value()) {
		ivec3 pos = opt_pos.value();
		oss << std::setw(12) << pos.x << ' ' <<
			std::setw(12) << pos.y << ' ' << std::setw(12) << pos.z;
	} else {
		oss << std::setw(12) << "None";
	}
	drawAtLine(oss.str(), 4);
}

void WindowTextDrawer::drawDirection(float pitch, float yaw) {
	std::ostringstream oss;
	oss << "Pitch / Yaw : " << std::setprecision(3) << std::fixed << std::setw(9) << pitch << ' ' <<
		std::fixed << std::setw(9) << yaw;
	drawAtLine(oss.str(), 5);
}

void WindowTextDrawer::drawBlockChunks(int blockChunks) {
	std::ostringstream oss;
	oss << "Block Chunks : " << std::setw(6) << blockChunks;
	drawAtLine(oss.str(), 6);
}

void WindowTextDrawer::drawFaceChunks(int faceChunks) {
	std::ostringstream oss;
	oss << "Face Chunks :  " << std::setw(6) << faceChunks;
	drawAtLine(oss.str(), 7);
}

void WindowTextDrawer::drawBlockNumber(int blockNumber) {
	std::ostringstream oss;
	oss << "Blocks : " << std::setw(12) << blockNumber;
	drawAtLine(oss.str(), 8);
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
