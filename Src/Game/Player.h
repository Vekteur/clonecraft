#pragma once

#include <optional>

#include <SFML/Window.hpp>

#include <View/Camera.h>
#include <Game/Movement.h>
#include <Block/Block.h>
#include <Commands/Commands.h>

class Game;

enum class GameMode {
	CREATIVE,
	SURVIVAL,
	SPECTATOR,
	SIZE
};

class Player {
public:

	Player(Game* game);

	void processMouseClick(sf::Time dt, Commands& commands);
	void processMouseMove(sf::Time dt);
	void processMouseWheel(sf::Time dt, GLfloat delta);
	void update(sf::Time dt);
	void teleport();
	void move(Movement::Direction direction, sf::Time dt);
	vec3 getPosition() const;
	void nextGameMode();
	void setGameMode(GameMode gameMode);
	GameMode getGameMode() const;

	Camera& getCamera();
	const Camera& getCamera() const;
	std::optional<ivec3> getTarget();

	static const float TARGET_DISTANCE;

	Game* game;
	GameMode m_gameMode = GameMode::CREATIVE;
	Camera m_camera;
	Movement m_movement;
	std::optional<ivec3> targetPos;
	std::optional<ivec3> placePos;
	std::optional<Block> pickedBlock;

	sf::Time breakAccumulator = sf::seconds(0.f);
	sf::Time placeAccumulator = sf::seconds(0.f);
};

