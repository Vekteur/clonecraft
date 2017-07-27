#pragma once

#include "Camera.h"
#include "ResManager.h"
#include "Chunk.h"

#include <memory>
#include <ChunkMap.h>

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
	ChunkMap m_chunks;

	GLboolean m_keys[1024];
};