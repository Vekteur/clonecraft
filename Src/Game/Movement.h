#pragma once

#include <functional>

#include <Maths/GlmCommon.h>
#include <Maths/SweptAABB.h>
#include <View/Camera.h>
#include <Block/Block.h>

class Player;

class Movement {
public:
	enum Direction {
		FORWARD,
		BACKWARD,
		RIGHT,
		LEFT,
		UP,
		DOWN
	};
	
	// Default values
	static const float HORIZONTAL_SPEED, VERTICAL_SPEED;
	static const float PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_HEAD_HEIGHT;

	Movement(const Player* player);

	void move(Direction direction, float deltaTime);
	void update(float deltaTime);
	vec3 getVelocityAndReset();
	vec3 getMoveAndReset(float deltaTime);
	vec3 getMoveWithCollisionsAndReset(float deltaTime);

private:
	const Player* p_player{ nullptr };

	float m_verticalSpeed{ 0.f };
	vec3 m_horizontalDir{};
	vec3 m_verticalDir{};
	bool m_onTheGround = false;
	bool m_inWater = false;

	std::vector<ivec3> getBroadphaseBlocks(const Box& hitbox, vec3 shift,
		std::function<bool(Block)> filter) const;
	Box makeHitbox() const;
	bool isInWater() const;
};