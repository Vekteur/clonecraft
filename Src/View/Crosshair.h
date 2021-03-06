#pragma once

#include "Window.h"
#include "Util/Logger.h"

class Crosshair
{
private:
	const float scale = 1.375f;
	Window* p_window;
	sf::Texture crossHairTexture;
	sf::Sprite crossHair;
public:
	Crosshair(Window* p_window) : p_window{ p_window } {
		if (!crossHairTexture.loadFromFile("Data/Textures/HUD/Crosshair.png")) {
			LOG(Level::ERROR) << "Failed to load crosshair texture" << std::endl;
		}
		crossHair.setTexture(crossHairTexture);
		crossHair.scale({ scale, scale });
	}
	
	void draw() {
		sf::FloatRect rect = crossHair.getGlobalBounds();
		ivec2 crossHairSize = { rect.width, rect.height };
		vec2 finalPos = p_window->getCenter() - crossHairSize / 2;
		crossHair.setPosition({ finalPos.x, finalPos.y });
		p_window->draw(crossHair);
	}
};