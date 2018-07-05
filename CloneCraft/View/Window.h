#pragma once

#include <glad/glad.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <GlmCommon.h>

#include <memory>

class Window
{
public:
	static const int SCREEN_WIDTH{ 1080 };
	static const int SCREEN_HEIGHT{ 720 };

	static const vec3 clearColor;

	Window();

	void toClose();
	bool shouldClose();
	void clear();
	void display();
	ivec2 getCenter();
	bool pollEvent(sf::Event& event);

private:
	sf::RenderWindow m_window;
	bool m_toClose{ false };

	void initSettings();
};