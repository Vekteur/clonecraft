#include "Game.h"

#include "Converter.h"
#include "Window.h"

Game::Game(Window* const window) : m_camera{ vec3{0.0f, 0.0f, 0.0f } }, p_window{ window }
{
	ResManager::loadShader("Resources/Shaders/cube.vs", "Resources/Shaders/cube.frag", nullptr, "cube");
	ResManager::loadTexture("Resources/Textures/stone.png", GL_FALSE, "stone");
	
	//m_chunkMapThread = std::thread{ &ChunkMap::load, &m_chunks, p_window->getGLFWChunkMapThreadWindow() };
	//m_chunkMapThread.join();
	m_chunks.load(p_window->getGLFWChunkMapThreadWindow());
}

Game::~Game()
{
	//m_chunkMapThread.join();
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

	/*ivec2 newCenter = Converter::globalToChunk(m_camera.getPosition());
	if (m_chunks.getCenter() != newCenter)
	{
		std::cout << "a";
		if (m_chunkMapThread.joinable())
		{
			std::cout << "chunkMapThread joined" << '\n';
			m_chunkMapThread.join();
			m_chunks.setCenter(newCenter);
			m_chunkMapThread = std::thread{ &ChunkMap::load, &m_chunks };
		}
	}*/
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
