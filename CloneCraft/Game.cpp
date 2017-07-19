#include "Game.h"

Game::Game() : m_camera{}, m_chunk{ glm::ivec2{0.0f, 0.0f} }
{
	ResManager::loadShader("Resources/Shaders/cube.vs", "Resources/Shaders/cube.frag", nullptr, "cube");
	ResManager::loadTexture("Resources/Textures/stone.png", GL_FALSE, "stone");

	m_chunk.load();
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
	m_chunk.render(ResManager::getShader("cube"), ResManager::getTexture("stone"));
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
