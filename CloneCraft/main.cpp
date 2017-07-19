#include <iostream>

#include "Window.h"
#include "Game.h"

int main()
{
	Window window;
	window.initCallbacks();

	GLfloat secondAccumulator = 0.0f;
	GLuint fps = 0;

	Game& game = window.getGame();

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

			std::cout << game.getCamera().getPosition() << '\n';
			std::cout << game.getCamera().getYaw() << ' ' << game.getCamera().getPitch() << '\n';
			
			secondAccumulator -= 1.0f;
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