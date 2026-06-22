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
	
	static const float WALK_HORIZONTAL_SPEED;
	static const float FLY_HORIZONTAL_SPEED, FLY_VERTICAL_SPEED;
	static const float WALK_SPRINT_MULTIPLIER, FLY_SPRINT_MULTIPLIER;
	static const float PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_HEAD_HEIGHT;

	static const float JUMP_SPEED;
	static const float GRAVITY;
	static const float WATER_JUMP_SPEED, WATER_SWIM_UP_ACCELERATION;
	static const float WATER_GRAVITY, WATER_HORIZONTAL_MULTIPLIER;
	static const float WATER_VERTICAL_DRAG;
	static const float PUSH_OUT_SPEED;

	Movement(const Player* player);

	void setSprinting(bool sprinting);
	void move(Direction direction, float deltaTime);
	void update(float deltaTime);
	vec3 getMoveAndReset(float deltaTime);
	bool isInWater() const;
	bool intersectsBlock(ivec3 blockPos) const;

private:
	const Player* p_player{ nullptr };

	float m_verticalSpeed{ 0.f };
	vec3 m_horizontalDir{};
	vec3 m_verticalDir{};
	bool m_onTheGround = false;
	bool m_inWater = false;
	bool m_sprinting = false;

	vec3 getVelocityAndReset();
	vec3 getMoveWithCollisionsAndReset(float deltaTime);
	vec3 getUnstuckShift(float deltaTime) const;
	std::vector<ivec3> getBroadphaseBlocks(const Box& hitbox, vec3 shift,
		std::function<bool(Block)> filter) const;
	Box makeHitbox() const;
};