#include "Window.h"

Window::Window()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CloneCraft", nullptr, nullptr);
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glfwSetWindowPos(window, (mode->width - width) / 2, (mode->height - height) / 2);
	glfwMakeContextCurrent(window);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glewExperimental = GL_TRUE;
	glewInit();
	glGetError();
}


Window::~Window()
{
	glfwTerminate();
}

bool Window::shouldClose()
{
	return glfwWindowShouldClose(window);
}

void Window::clear()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::swapBuffers()
{
	glfwSwapBuffers(window);
}

void Window::initCallbacks()
{
	glfwSetKeyCallback(window, Window::key_callback);
	glfwSetCursorPosCallback(window, Window::mouse_callback);
	glfwSetScrollCallback(window, Window::scroll_callback);
}

void Window::pollEvents()
{
	glfwPollEvents();
}

void Window::key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void Window::mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
}

void Window::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
}