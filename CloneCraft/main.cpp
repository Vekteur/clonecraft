#include <iostream>

#include "Window.h"
#include "Game.h"
#include "Debug.h"

int main()
{
	Window window;
	window.initCallbacks();

	GLfloat accumulator = 0.0f;
	GLuint fps = 0;

	Game game{ &window };

	GLfloat lastFrame = 0.0f;
	while (!window.shouldClose())
	{
		// Calculate delta time of the current frame
		GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
		GLfloat deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// show FPS
		accumulator += deltaTime;
		++fps;
		if (accumulator >= 1.0f)
		{
			std::cout << fps << std::endl;
			
			accumulator -= 1.0f;
			fps = 0;
		}

		window.pollEvents();

		game.processInput(deltaTime);
		game.update(deltaTime);

		window.clear();

		game.render();

		window.swapBuffers();
	}

	return 0;
}