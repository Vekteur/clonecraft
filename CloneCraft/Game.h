#pragma once

#include "Camera.h"
#include "ResManager.h"
#include "Chunk.h"

#include <memory>

class Game
{
public:
	Game();
	~Game();

	void processInput(GLfloat dt);
	void update(GLfloat dt);
	void render();

	Camera& getCamera();
	void setKey(int key, GLboolean enable);

private:
	Camera m_camera;
	std::unique_ptr<Chunk> m_chunks[10][10];

	GLboolean m_keys[1024];
};