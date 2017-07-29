#include "Game.h"

#include "Converter.h"
#include "Window.h"
#include "Debug.h"

Game::Game(Window* const window) : m_camera{ vec3{0.0f, 0.0f, 0.0f } }, p_window{ window }
{
	ResManager::loadShader("Resources/Shaders/cube.vs", "Resources/Shaders/cube.frag", nullptr, "cube");
	ResManager::loadTexture("Resources/Textures/stone.png", GL_FALSE, "stone");
	
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

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}

	while (!stopChunkMapThread)
	{
		m_chunks.load();
	}
}

void Game::processInput(GLfloat dt)
{
	if (m_keys[GLFW_KEY_W])
		m_camera.move(Camera::FORWARD, dt);
	if (m_keys[GLFW_KEY_S])
		m_camera.move(Camera::BACKWARD, dt);
	if (m_keys[GLFW_KEY_A])
		m_camera.move(Camera::LEFT, dt);
	if (m_keys[GLFW_KEY_D])
		m_camera.move(Camera::RIGHT, dt);
}

void Game::update(GLfloat dt)
{
	ResManager::getShader("cube").use().setMatrix4("projection", m_camera.getProjectionMatrix());
	ResManager::getShader("cube").use().setMatrix4("view", m_camera.getViewMatrix());

	ivec2 newCenter = Converter::globalToChunk(m_camera.getPosition());
	if (m_chunks.getCenter() != newCenter)
		m_chunks.setCenter(newCenter);
}

void Game::render()
{
	m_chunks.render();
}

Camera& Game::getCamera()
{
	return m_camera;
}

void Game::setKey(int key, GLboolean enable)
{
	assert(0 <= key && key < 1024);
	m_keys[key] = enable;
}
