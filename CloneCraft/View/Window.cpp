#include "Window.h"
#include "Keyboard.h"
#include "Mouse.h"

const vec3 Window::clearColor{ 70.f / 255, 190.f / 255, 240.f / 255 };

Window::Window() {
	// Init glfw
	assert(glfwInit());

	initChunkMapWindow();
	initMainWindow();

	// Init GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}

	// OpenGL settings
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

Window::~Window() {
	glfwTerminate();
}

bool Window::shouldClose() {
	return glfwWindowShouldClose(mainWindow);
}

void Window::close() {
	glfwSetWindowShouldClose(mainWindow, GL_TRUE);
}

void Window::clear() {
	glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::swapBuffers() {
	glfwSwapBuffers(mainWindow);
}

void Window::initCallbacks() {
	glfwSetKeyCallback(mainWindow, Window::key_callback);
	glfwSetCursorPosCallback(mainWindow, Window::mouse_callback);
	glfwSetScrollCallback(mainWindow, Window::scroll_callback);
}

void Window::pollEvents() {
	glfwPollEvents();
}

GLFWwindow * Window::getGLFWMainWindow() {
	return mainWindow;
}

GLFWwindow * Window::getGLFWChunkMapThreadWindow() {
	return chunkMapWindow;
}

void Window::initChunkMapWindow() {
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	chunkMapWindow = glfwCreateWindow(1, 1, "ChunkMap Thread", nullptr, nullptr);
}

void Window::initMainWindow() {
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	// Share OpenGL resources of mainWindow with chunkMapThread
	mainWindow = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CloneCraft", nullptr, chunkMapWindow);
	// Center the window
	int width, height;
	glfwGetFramebufferSize(mainWindow, &width, &height);
	glfwSetWindowPos(mainWindow, (mode->width - width) / 2, (mode->height - height) / 2);
	glfwMakeContextCurrent(mainWindow);

	glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
	if (0 <= key && key < 1024) {
		if (action == GLFW_PRESS)
			Keyboard::setKey(key, GL_TRUE);
		else if (action == GLFW_RELEASE)
			Keyboard::setKey(key, GL_FALSE);
	}
}

void Window::mouse_callback(GLFWwindow *window, double xpos, double ypos) {
	Mouse::setPosition(vec2{ static_cast<float>(xpos), static_cast<float>(ypos) });
}

void Window::scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
	Mouse::addScrolling(vec2{ static_cast<float>(xoffset), static_cast<float>(yoffset) });
}