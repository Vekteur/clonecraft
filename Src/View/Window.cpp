#include "Window.h"

#include "Util/Logger.h"
#include "Util/Debug.h"

const vec3 Window::clearColor{ 70.f / 255, 190.f / 255, 240.f / 255 };

Window::Window() : sf::RenderWindow{} {
	sf::ContextSettings settings;
	settings.majorVersion = 4;
	settings.minorVersion = 3;
	settings.depthBits = 24;
	settings.stencilBits = 8;
#ifdef _DEBUG
	settings.attributeFlags = sf::ContextSettings::Attribute::Debug;
#endif
	this->create(sf::VideoMode{ SCREEN_WIDTH, SCREEN_HEIGHT }, "CloneCraft", sf::Style::Default, settings);

	// Init GLAD
	if (!gladLoadGL()) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}

#ifdef _DEBUG
	initDebug();
#endif
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

ivec2 Window::size() const {
	return { this->getSize().x, this->getSize().y };
}

ivec2 Window::getCenter() {
	return { this->getSize().x / 2, this->getSize().y / 2 };
}

vec3 Window::getClearColor() {
	return clearColor;
}

void Window::initDebug() {
	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(Debug::glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		LOG(Level::INFO) << "Debug context initialized" << std::endl;
	}
}

void Window::initSettings() {
	// OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glEnable(GL_CLIP_DISTANCE0);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}