#include "Game.h"

#include "Converter.h"
#include "Window.h"
#include "Debug.h"
#include "Keyboard.h"
#include "Mouse.h"

Game::Game(Window* const window) : m_camera{ vec3{0.0f, 80.0f, 0.0f } }, p_window{ window }
{
	ResManager::loadShader("Resources/Shaders/cube.vs", "Resources/Shaders/cube.frag", nullptr, "cube");
	ResManager::loadTexture("Resources/Textures/stone.png", GL_FALSE, "stone");

	ResManager::getShader("cube").use().setInteger("distance", ChunkMap::VIEW_DISTANCE);

	if (glGetError())
		std::cin.get();
	
	m_chunkMapThread = std::thread{ &Game::runChunkLoadingLoop, this };
}

Game::~Game()
{
	stopChunkMapThread = true;
	m_chunkMapThread.join();
}

void Game::runChunkLoadingLoop()
{
	glfwMakeContextCurrent(p_window->getGLFWChunkMapThreadWindow());

	while (!stopChunkMapThread)
	{
		m_chunks.load();
		m_chunks.unloadFarChunks();
	}
}

void Game::processInput(GLfloat dt)
{
	if (Keyboard::isKeyPressed(GLFW_KEY_W))
		m_camera.move(Camera::FORWARD, dt);
	if (Keyboard::isKeyPressed(GLFW_KEY_S))
		m_camera.move(Camera::BACKWARD, dt);
	if (Keyboard::isKeyPressed(GLFW_KEY_A))
		m_camera.move(Camera::LEFT, dt);
	if (Keyboard::isKeyPressed(GLFW_KEY_D))
		m_camera.move(Camera::RIGHT, dt);
	if (Keyboard::isKeyPressed(GLFW_KEY_ESCAPE))
		p_window->close();

	vec2 mouseOffset{ Mouse::getPosition().x - m_lastMousePosition.x, m_lastMousePosition.y - Mouse::getPosition().y };
	m_lastMousePosition = Mouse::getPosition();
	m_camera.processMouse(mouseOffset);

	m_camera.processMouseScroll(Mouse::getScrolling().y);
	Mouse::resetScrolling();
}

void Game::update(GLfloat dt)
{
	ResManager::getShader("cube").use().setMatrix4("view", m_camera.getViewMatrix());
	ResManager::getShader("cube").use().setMatrix4("projection", m_camera.getProjectionMatrix());
	ResManager::getShader("cube").use().setVector3f("skyColor", p_window->clearColor);

	ivec2 newCenter = Converter::globalToChunk(m_camera.getPosition());
	if (m_chunks.getCenter() != newCenter)
	{
		m_chunks.setCenter(newCenter);
	}
		
}

void Game::render()
{
	m_chunks.render();
}

Camera& Game::getCamera()
{
	return m_camera;
}