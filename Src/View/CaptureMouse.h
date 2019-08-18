#pragma once

#include "Window.h"

class CaptureMouse {
private:
	bool enabled = false;
	Window* const p_window{ nullptr };
public:
	CaptureMouse(Window* const p_window) : p_window{ p_window } {
		toggle();
	}
	void toggle() {
		enabled = !enabled;
		p_window->setMouseCursorVisible(!enabled);
		update();
	}
	void update() {
		if (enabled) {
			sf::Mouse::setPosition(sf::Vector2i{ p_window->getCenter().x, p_window->getCenter().y }, *p_window);
		}
	}
	bool isEnabled() {
		return enabled;
	}
};