#pragma once

#include <glad\glad.h>
#include <GLFW/glfw3.h>
#include <GlmCommon.h>

#include "Game.h"

#include <memory>

class Window
{
public:
	static const int SCREEN_WIDTH{ 1080 };
	static const int SCREEN_HEIGHT{ 720 };

	static const vec3 clearColor;

	Window();
	~Window();

	bool shouldClose();
	void clear();
	void swapBuffers();
	void initCallbacks();
	void pollEvents();

	GLFWwindow* getGLFWMainWindow();
	GLFWwindow* getGLFWChunkMapThreadWindow();

	Game& getGame();

private:
	GLFWwindow *mainWindow, *chunkMapThread;
	static std::unique_ptr<Game> m_game;

	static bool firstMouse;
	static double lastX, lastY;

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};