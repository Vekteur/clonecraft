#include <iostream>
#include <glad/glad.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>

#include "Game.h"
#include "Debug.h"
#include "Window.h"
#include "Logger.h"
#include "FPSCounter.h"
#include "WorldConstants.h"
#include "WindowTextDrawer.h"

int main(int argc, char* argv[]) {
	LOG.setFileOutputLevel(Level::DEBUG);
	LOG.setOutputFile("Logs/global.log");

	LOG(Level::INFO) << "Application launched" << std::endl;
	
	sf::Context context;
	Window window;
	Game game{ &window, &context };

	WindowTextDrawer textDrawer{ &window };
	
	sf::Clock clock;
	FPSCounter fpsCounter;

	while (!window.shouldClose()) {
		sf::Time deltaTime = clock.getElapsedTime();
		clock.restart();
		fpsCounter.update(deltaTime);

		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Escape)
					window.toClose();
				break;
			case sf::Event::Closed:
				window.toClose();
				break;
			case sf::Event::Resized:
				glViewport(0, 0, event.size.width, event.size.height);
				break;
			case sf::Event::MouseWheelScrolled:
				if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
					game.processMouseWheel(event.mouseWheelScroll.delta);
				}
				break;
			}
		}
		
		game.processInput(deltaTime.asSeconds());
		game.update(deltaTime.asSeconds());
		
		window.clear();
		game.render();

		window.pushGLStates();

		textDrawer.drawFPS(fpsCounter.get());
		textDrawer.drawPosition(game.getCamera().getPosition());
		textDrawer.drawDirection(game.getCamera().getYaw(), game.getCamera().getPitch());
		textDrawer.drawChunksInfos(game.getChunkMap().chunksAtLeastInState(Chunk::TO_LOAD_FACES), 
			game.getChunkMap().chunksAtLeastInState(Chunk::TO_LOAD_VAOS));
		textDrawer.drawBlockNumber(game.getChunkMap().chunksAtLeastInState(Chunk::TO_LOAD_FACES) * 
			Const::CHUNK_SIDE * Const::CHUNK_SIDE * Const::CHUNK_HEIGHT);

		window.popGLStates();

		window.display();
	}
	
	LOG(Level::INFO) << "Application terminated" << std::endl;
	return 0;
}