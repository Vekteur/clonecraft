#include "Game.h"

Game::Game() : m_camera{ vec3{0.0f, 0.0f, 0.0f } }
{
	ResManager::loadShader("Resources/Shaders/cube.vs", "Resources/Shaders/cube.frag", nullptr, "cube");
	ResManager::loadTexture("Resources/Textures/stone.png", GL_FALSE, "stone");

	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			m_chunks[i][j] = std::move(std::make_unique<Chunk>(glm::vec2{i, j}));
			m_chunks[i][j]->load();
		}
	}
}

Game::~Game()
{
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
}

void Game::render()
{
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++)
			m_chunks[i][j]->render(ResManager::getShader("cube"), ResManager::getTexture("stone"));
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
