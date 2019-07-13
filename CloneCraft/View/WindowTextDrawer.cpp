#include "WindowTextDrawer.h"

#include <iomanip>

#include "Generator/WorldGenerator.h"
#include "Util/Logger.h"
#include "Maths/Converter.h"

WindowTextDrawer::WindowTextDrawer(Window * window) : p_window{ window } {
	if (!m_font.loadFromFile("Resources/Fonts/arial.ttf")) {
		LOG(Level::ERROR) << "Failed to load font" << std::endl;
	}
}

void WindowTextDrawer::drawAll(int fps, Game& game) {
	line = 0;
	drawFPS(fps);
	vec3 pos = game.getCamera().getPosition();
	ivec3 blockPos = Converter::globalPosToBlock(pos);
	drawGlobalPosition(pos);
	drawLocalPosition(Converter::globalToInnerSection(pos));
	drawSectionPosition(Converter::globalToSection(blockPos));
	drawTarget(game.getTarget());
	drawDirection(game.getCamera().getYaw(), game.getCamera().getPitch());
	drawBiome(g_worldGenerator.biomeMap().getBiomeName(Converter::to2D(blockPos)));
	drawRenderedChunks(game.getChunkMap().getRenderedChunks());
	drawBlockChunks(game.getChunkMap().chunksAtLeastInState(Chunk::TO_LOAD_FACES));
	drawFaceChunks(game.getChunkMap().chunksAtLeastInState(Chunk::TO_RENDER));
	/*drawBlockNumber(game.getChunkMap().chunksAtLeastInState(Chunk::TO_LOAD_FACES) *
		Const::CHUNK_SIDE * Const::CHUNK_SIDE * Const::CHUNK_HEIGHT);*/
}

void WindowTextDrawer::drawFPS(int fps) {
	std::ostringstream oss;
	oss << "FPS : " << std::setw(4) << fps;
	draw(oss.str());
}

void WindowTextDrawer::drawGlobalPosition(vec3 pos) {
	std::ostringstream oss;
	oss << "Global :  " << std::setprecision(2) << std::fixed << std::setw(10) << std::right << pos.x << ' ' <<
		std::fixed << std::setw(10) << std::right << pos.y << ' ' << std::fixed << std::setw(10) << std::right << pos.z;
	draw(oss.str());
}

void WindowTextDrawer::drawLocalPosition(vec3 pos) {
	std::ostringstream oss;
	oss << "Local :   " << std::setprecision(4) << std::fixed << std::setw(8) << pos.x << ' ' <<
		std::fixed << std::setw(8) << pos.y << ' ' << std::fixed << std::setw(8) << pos.z;
	draw(oss.str());
}

void WindowTextDrawer::drawSectionPosition(ivec3 pos) {
	std::ostringstream oss;
	oss << "Section : " << std::setw(12) << pos.x << ' ' <<
		std::setw(12) << pos.y << ' ' << std::setw(12) << pos.z;
	draw(oss.str());
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
	draw(oss.str());
}

void WindowTextDrawer::drawDirection(float pitch, float yaw) {
	std::ostringstream oss;
	oss << "Pitch / Yaw : " << std::setprecision(3) << std::fixed << std::setw(9) << pitch << ' ' <<
		std::fixed << std::setw(9) << yaw;
	draw(oss.str());
}

void WindowTextDrawer::drawBiome(std::string biomeName) {
	draw("Biome : " + biomeName);
}

void WindowTextDrawer::drawRenderedChunks(int renderedChunks) {
	std::ostringstream oss;
	oss << "Rendered Chunks : " << std::setw(6) << renderedChunks;
	draw(oss.str());
}

void WindowTextDrawer::drawBlockChunks(int blockChunks) {
	std::ostringstream oss;
	oss << "Block Chunks : " << std::setw(6) << blockChunks;
	draw(oss.str());
}

void WindowTextDrawer::drawFaceChunks(int faceChunks) {
	std::ostringstream oss;
	oss << "Face Chunks :  " << std::setw(6) << faceChunks;
	draw(oss.str());
}

void WindowTextDrawer::drawBlockNumber(int blockNumber) {
	std::ostringstream oss;
	oss << "Blocks : " << std::setw(12) << blockNumber;
	draw(oss.str());
}

void WindowTextDrawer::draw(std::string message) {
	sf::Text text;
	text.setFont(m_font);
	text.setString(message);
	text.setCharacterSize(static_cast<unsigned int>(textSize * p_window->getSize().y));
	text.setPosition({ paddingLeft * p_window->getSize().x, 
		p_window->getSize().y * (paddingTop + line * textSpacing) });
	p_window->draw(text);
	++line;
}
