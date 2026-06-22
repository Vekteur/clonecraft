#pragma once

#include <optional>

#include <SFML/Window.hpp>

#include <View/Camera.h>
#include <Game/Movement.h>
#include <Block/Block.h>
#include <Block/BlockID.h>
#include <Commands/Commands.h>

class Game;

enum class GameMode {
	SURVIVAL,
	CREATIVE,
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
	void placeBlockBelow();
	void move(Movement::Direction direction, sf::Time dt);
	void setSprinting(bool sprinting);
	vec3 getPosition() const;
	void nextGameMode();
	void setGameMode(GameMode gameMode);
	GameMode getGameMode() const;

	Game& getGame();
	const Game& getGame() const;
	Camera& getCamera();
	const Camera& getCamera() const;
	std::optional<ivec3> getTarget() const;
	std::optional<Block> getPickedBlock() const;
	bool isInWater() const;
	bool intersectsBlock(ivec3 blockPos) const;

private:
	static const vec3 INITIAL_POSITION;
	static const float DEFAULT_TARGET_DISTANCE, CREATIVE_TARGET_DISTANCE;

	Game* game;
	GameMode m_gameMode = GameMode::SURVIVAL;
	Camera m_camera;
	Movement m_movement;
	std::optional<ivec3> targetPos;
	std::optional<ivec3> placePos;
	std::optional<Block> pickedBlock{ BlockID::WATER };

	sf::Time breakAccumulator = sf::seconds(0.f);
	sf::Time placeAccumulator = sf::seconds(0.f);
};

