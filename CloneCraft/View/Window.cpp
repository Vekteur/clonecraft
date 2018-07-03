#include "Window.h"

const vec3 Window::clearColor{ 70.f / 255, 190.f / 255, 240.f / 255 };

Window::Window() {
	sf::ContextSettings settings;
	settings.majorVersion = 3;
	settings.minorVersion = 3;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	window.create(sf::VideoMode{ 1080, 720 }, "CloneCraft", sf::Style::Default, settings);
	window.setMouseCursorVisible(false);

	// Init GLAD
	if (!gladLoadGL()) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}

	initSettings();
}

void Window::close() {
	toClose = true;
}

bool Window::shouldClose() {
	return toClose;
}

void Window::clear() {
	glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::display() {
	window.display();
}

ivec2 Window::getCenter() {
	return { window.getSize().x / 2, window.getSize().y / 2 };
}

bool Window::pollEvent(sf::Event & event) {
	return window.pollEvent(event);
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