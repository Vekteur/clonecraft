#pragma once

#include <glad/glad.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <GlmCommon.h>

#include <memory>

class Window : public sf::RenderWindow
{
public:
	static const unsigned int SCREEN_WIDTH{ 1080 };
	static const unsigned int SCREEN_HEIGHT{ 720 };

	static const vec3 clearColor;

	Window();

	void toClose();
	bool shouldClose();
	void clear();
	ivec2 getCenter();

private:
	bool m_toClose{ false };

	void initSettings();
};