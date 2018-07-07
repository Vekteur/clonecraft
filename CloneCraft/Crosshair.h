#pragma once

#include "Window.h"
#include "Logger.h"

class Crosshair
{
private:
	const float scale = 1.375f;
	Window* p_window;
	sf::Texture crossHairTexture;
	sf::Sprite crossHair;
public:
	Crosshair(Window* p_window) : p_window{ p_window } {
		if (!crossHairTexture.loadFromFile("Resources/Textures/Crosshair.png")) {
			LOG(Level::ERROR) << "Failed to load crosshair texture" << std::endl;
		}
		crossHair.setTexture(crossHairTexture);
		crossHair.scale({ scale, scale });
		sf::FloatRect rect = crossHair.getGlobalBounds();
		ivec2 crossHairSize = { rect.width, rect.height };
		vec2 finalPos = p_window->getCenter() - crossHairSize / 2;
		crossHair.setPosition({ finalPos.x, finalPos.y });
	}
	
	void draw() {
		p_window->draw(crossHair);
	}
};