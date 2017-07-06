#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Window.h"

int main()
{
	Window window;
	window.initCallbacks();

	GLfloat secondAccumulator = 0.0f;
	GLuint fps = 0;

	//CloneCraft.init();

	GLfloat lastFrame = 0.0f;

	while (!window.shouldClose())
	{
		// Calculate delta time of the current frame
		GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
		GLfloat deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// show FPS
		secondAccumulator += deltaTime;
		fps++;
		if (secondAccumulator >= 1.0f)
		{
			std::cout << fps << std::endl;
			
			secondAccumulator -= 1.0f;
			fps = 0;
		}

		window.pollEvents();

		//CloneCraft.processInput(deltaTime);
		//CloneCraft.update(deltaTime);

		window.clear();

		//CloneCraft.render();

		window.swapBuffers();
	}

	return 0;
}