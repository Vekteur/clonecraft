#pragma once

#include <glad/glad.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <Maths/GlmCommon.h>

#include <memory>

class Window : public sf::RenderWindow
{
public:
	static const unsigned int SCREEN_WIDTH{ 1366 };
	static const unsigned int SCREEN_HEIGHT{ 768 };

	static const vec3 clearColor;

	Window();

	void toClose();
	bool shouldClose();
	void clear();
	ivec2 size() const;
	ivec2 getCenter();
	vec3 getClearColor();

private:
	bool m_toClose{ false };

	void initDebug();
	void initSettings();
};