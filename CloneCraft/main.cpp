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
#include "CaptureMouse.h"
#include "Crosshair.h"

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

int main(int argc, char* argv[]) {
	LOG.setFileOutputLevel(Level::DEBUG);
	LOG.setOutputFile("Logs/global.log");

	LOG(Level::INFO) << "Application launched" << std::endl;
	sf::Context context;
	Window window;
	Game game{ &window, &context };
	CaptureMouse captureMouse{ &window };

	WindowTextDrawer textDrawer{ &window };
	Crosshair crosshair{ &window };
	
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
				if (event.key.code == sf::Keyboard::Escape) {
					captureMouse.toggle();
				}
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

		game.processKeyboard(deltaTime.asSeconds());
		if (captureMouse.isEnabled())
			game.processMouseMove(deltaTime.asSeconds());
		game.update(deltaTime.asSeconds());
		captureMouse.update();

		window.clear();
		game.render();

		window.pushGLStates();
		crosshair.draw();
		textDrawer.draw(fpsCounter.get(), game);
		window.popGLStates();

		window.display();
	}
	
	LOG(Level::INFO) << "Application terminated" << std::endl;
	return 0;
}