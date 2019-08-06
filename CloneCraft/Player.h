#pragma once

#include <optional>

#include <SFML/Window.hpp>

#include "View/Camera.h"
#include "Block/Block.h"
#include "Commands/Commands.h"

class Game;

class Player {
public:
	Player(Game* game);

	void processMouseClick(sf::Time dt, Commands& commands);
	void processMouseMove(sf::Time dt);
	void processMouseWheel(sf::Time dt, GLfloat delta);
	void update(sf::Time dt);
	void teleport();
	void move(Camera::Direction direction, float deltaTime);
	vec3 getPosition();

	Camera& getCamera();
	std::optional<ivec3> getTarget();

	static const float TARGET_DISTANCE;

	Game* game;
	Camera m_camera;
	std::optional<ivec3> targetPos;
	std::optional<ivec3> placePos;
	std::optional<Block> pickedBlock;

	sf::Time breakAccumulator = sf::seconds(0.f);
	sf::Time placeAccumulator = sf::seconds(0.f);
};

