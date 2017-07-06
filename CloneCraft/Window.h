#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window
{
public:
	static const int SCREEN_WIDTH{ 1080 };
	static const int SCREEN_HEIGHT{ 720 };

	Window();
	~Window();

	bool shouldClose();
	void clear();
	void swapBuffers();
	void initCallbacks();
	void pollEvents();

private:
	GLFWwindow* window;

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};
