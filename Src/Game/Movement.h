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

	static const float PUSH_OUT_SPEED;

	Movement(const Player* player);

	void setSprinting(bool sprinting);
	void move(Direction direction, float deltaTime);
	void update(float deltaTime);
	vec3 getMoveAndReset(float deltaTime);
	bool isInWater() const;
	bool intersectsBlock(ivec3 blockPos) const;

private:
	enum class Fluid { NONE, WATER, LAVA };
	struct FluidPhysics {
		float jumpSpeed;
		float swimUpAcceleration;
		float gravity;
		float horizontalMultiplier;
		float verticalDrag; // fraction of vertical speed kept per second
	};
	// Lava is thicker than water: you barely climb out, sink slowly and move sluggishly.
	static constexpr FluidPhysics WATER_PHYSICS{ 7.f, 20.f, 10.f, 0.6f, 0.07f };
	static constexpr FluidPhysics LAVA_PHYSICS{ 4.f, 12.f, 6.f, 0.35f, 0.04f };
	static constexpr FluidPhysics fluidPhysics(Fluid fluid) {
		return fluid == Fluid::LAVA ? LAVA_PHYSICS : WATER_PHYSICS;
	}

	const Player* p_player{ nullptr };

	float m_verticalSpeed{ 0.f };
	vec3 m_horizontalDir{};
	vec3 m_verticalDir{};
	bool m_onTheGround = false;
	Fluid m_fluid = Fluid::NONE;
	bool m_sprinting = false;

	// The fluid the hitbox currently overlaps; lava wins if it somehow touches both.
	Fluid currentFluid() const;
	vec3 getVelocityAndReset();
	vec3 getMoveWithCollisionsAndReset(float deltaTime);
	vec3 getUnstuckShift(float deltaTime) const;
	std::vector<ivec3> getBroadphaseBlocks(const Box& hitbox, vec3 shift,
		std::function<bool(Block)> filter) const;
	Box makeHitbox() const;
};