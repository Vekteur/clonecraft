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
#include "Commands.h"

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

int main(int argc, char* argv[]) {
	LOG.setFileOutputLevel(Level::DEBUG);
	LOG.setOutputFile("Logs/global.log");

	LOG(Level::INFO) << "Application launched" << std::endl;
	sf::Context context1;
	sf::Context context2;
	Window window;
	Game game{ &window, &context1, &context2 };
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
		Commands commands;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::Closed:
				window.toClose();
				break;
			case sf::Event::Resized:
				game.onChangedSize({ event.size.width, event.size.height });
				break;
			case sf::Event::KeyPressed:
				commands.onPressedEvent(event.key.code);
				if (event.key.code == sf::Keyboard::Escape) {
					captureMouse.toggle();
				}
				break;
			case sf::Event::MouseButtonPressed:
				commands.onPressedEvent(event.mouseButton.button);
				break;
			case sf::Event::MouseWheelScrolled:
				if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
					game.processMouseWheel(deltaTime, event.mouseWheelScroll.delta);
				}
				break;
			}
		}

		if (captureMouse.isEnabled()) {
			game.processMouseMove(deltaTime);
			game.processKeyboard(deltaTime, commands);
			game.processMouseClick(deltaTime, commands);
		}
		game.update(deltaTime);
		captureMouse.update();

		game.render();

		window.pushGLStates();
		crosshair.draw();
		textDrawer.drawAll(fpsCounter.get(), game);
		window.popGLStates();

		window.display();
	}
	
	LOG(Level::INFO) << "Application terminated" << std::endl;
	return 0;
}