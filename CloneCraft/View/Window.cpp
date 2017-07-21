#include "Window.h"

double Window::lastX{ SCREEN_WIDTH / 2 };
double Window::lastY{ SCREEN_HEIGHT / 2 };
bool Window::firstMouse{ GL_FALSE };
std::unique_ptr<Game> Window::m_game{};

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
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glewExperimental = GL_TRUE;
	glewInit();
	glGetError();

	m_game = std::move(std::make_unique<Game>());
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
	glClearColor(70.0f / 255, 190.0f / 255, 240.0f / 255, 1.0f);
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

Game& Window::getGame()
{
	return *m_game;
}

void Window::key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			m_game->setKey(key, GL_TRUE);
		else if (action == GLFW_RELEASE)
			m_game->setKey(key, GL_FALSE);
	}
}

void Window::mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	m_game->getCamera().processMouse(xoffset, yoffset);
}

void Window::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	m_game->getCamera().processMouseScroll(yoffset);
}