#pragma once

#include <SFML/System.hpp>

#include "Util/Logger.h"

class FPSCounter {
private:
	const sf::Time countTime = sf::seconds(1.f);
	sf::Time accumulator{ sf::seconds(0.f) };
	int fps = 0;
	int lastFps = 0;

public:
	void update(sf::Time deltaTime) {
		accumulator += deltaTime;
		++fps;
		if (accumulator >= countTime) {
			LOG(Level::INFO) << "FPS : " << fps << std::endl;
			accumulator -= countTime;
			lastFps = fps;
			fps = 0;
		}
	}

	int get() {
		return lastFps;
	}
};