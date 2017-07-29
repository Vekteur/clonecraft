#include "Window.h"

double Window::lastX{ SCREEN_WIDTH / 2 };
double Window::lastY{ SCREEN_HEIGHT / 2 };
const vec3 Window::clearColor{ 70.0f / 255, 190.0f / 255, 240.0f / 255 };
bool Window::firstMouse{ GL_FALSE };
std::unique_ptr<Game> Window::m_game{};

Window::Window()
{
	// Init glfw
	assert(glfwInit());

	// chunkMapThread context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	chunkMapThread = glfwCreateWindow(1, 1, "ChunkMap Thread", nullptr, nullptr);

	// mainWindow context
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	// Share OpenGL resources of mainWindow with chunkMapThread
	mainWindow = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CloneCraft", nullptr, chunkMapThread);
	// Center the window
	int width, height;
	glfwGetFramebufferSize(mainWindow, &width, &height);
	glfwSetWindowPos(mainWindow, (mode->width - width) / 2, (mode->height - height) / 2);
	glfwMakeContextCurrent(mainWindow);

	glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Init GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
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

	m_game = std::move(std::make_unique<Game>(this));
}


Window::~Window()
{
	glfwTerminate();
}

bool Window::shouldClose()
{
	return glfwWindowShouldClose(mainWindow);
}

void Window::clear()
{
	glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::swapBuffers()
{
	glfwSwapBuffers(mainWindow);
}

void Window::initCallbacks()
{
	glfwSetKeyCallback(mainWindow, Window::key_callback);
	glfwSetCursorPosCallback(mainWindow, Window::mouse_callback);
	glfwSetScrollCallback(mainWindow, Window::scroll_callback);
}

void Window::pollEvents()
{
	glfwPollEvents();
}

GLFWwindow * Window::getGLFWMainWindow()
{
	return mainWindow;
}

GLFWwindow * Window::getGLFWChunkMapThreadWindow()
{
	return chunkMapThread;
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