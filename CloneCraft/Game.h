#pragma once

#include "Camera.h"
#include "ResManager.h"
#include "Chunk.h"

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
	Chunk m_chunk;

	GLboolean m_keys[1024];
};