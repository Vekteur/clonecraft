#include <iostream>
#include <glad/glad.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#include "Game.h"
#include "Debug.h"
#include "Window.h"
#include "Logger.h"

class FPSCounter {
private:
	const sf::Time countTime = sf::seconds(1.f);
	sf::Time accumulator{ sf::seconds(0.f) };
	int fps = 0;

public:
	void update(sf::Time deltaTime) {
		accumulator += deltaTime;
		++fps;
		if (accumulator >= countTime) {
			LOG(Level::INFO) << "FPS : " << fps << std::endl;
			accumulator -= countTime;
			fps = 0;
		}
	}
};
int main(int argc, char* argv[]) {
	LOG.setFileOutputLevel(Level::DEBUG);
	LOG.setOutputFile("Logs/global.log");

	LOG(Level::INFO) << "Application launched" << std::endl;
	
	sf::Context context;

	Window window;

	Game game{ &window, &context };
	
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

		window.display();
	}
	
	LOG(Level::INFO) << "Application terminated" << std::endl;
	return 0;
}