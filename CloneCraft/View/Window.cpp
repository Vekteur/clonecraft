#include "Window.h"

const vec3 Window::clearColor{ 70.f / 255, 190.f / 255, 240.f / 255 };

Window::Window() : sf::RenderWindow{} {
	sf::ContextSettings settings;
	settings.majorVersion = 3;
	settings.minorVersion = 3;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	this->create(sf::VideoMode{ SCREEN_WIDTH, SCREEN_HEIGHT }, "CloneCraft", sf::Style::Default, settings);
	this->setMouseCursorVisible(false);

	// Init GLAD
	if (!gladLoadGL()) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}

	initSettings();
}

void Window::toClose() {
	m_toClose = true;
}

bool Window::shouldClose() {
	return m_toClose;
}

void Window::clear() {
	glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

ivec2 Window::getCenter() {
	return { this->getSize().x / 2, this->getSize().y / 2 };
}

void Window::initSettings() {
	// OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}