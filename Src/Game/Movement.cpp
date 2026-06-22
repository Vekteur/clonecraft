#include "Movement.h"

#include <cmath>

#include <Game/Player.h>
#include <Game/Game.h>
#include "Maths/AABB.h"

const float Movement::WALK_HORIZONTAL_SPEED{ 5.f };
const float Movement::FLY_HORIZONTAL_SPEED{ 15.f };
const float Movement::FLY_VERTICAL_SPEED{ 15.f };
const float Movement::WALK_SPRINT_MULTIPLIER{ 1.85f };
const float Movement::FLY_SPRINT_MULTIPLIER{ 20.f };
const float Movement::PLAYER_WIDTH{ 0.6f };
const float Movement::PLAYER_HEIGHT{ 1.8f };
const float Movement::PLAYER_HEAD_HEIGHT{ 1.65f };

const float Movement::JUMP_SPEED{ 10.f };
const float Movement::GRAVITY{ 32.f };
const float Movement::WATER_JUMP_SPEED{ 7.f };
const float Movement::WATER_SWIM_UP_ACCELERATION{ 20.f };
const float Movement::WATER_GRAVITY{ 10.f };
const float Movement::WATER_HORIZONTAL_MULTIPLIER{ 0.6f };
const float Movement::WATER_VERTICAL_DRAG{ 0.07f }; // Fraction of vertical speed kept per second in water
const float Movement::PUSH_OUT_SPEED{ 8.f }; // Blocks per second the player is ejected when stuck inside a block

Movement::Movement(const Player* player)
	: p_player(player)
{ }

void Movement::setSprinting(bool sprinting) {
	m_sprinting = sprinting;
}

vec3 toHorizontal(vec3 vec) { // Projects the vector on the XZ plane
	return normalize(vec3{ vec.x, 0.f, vec.z });
}

void Movement::move(Direction direction, float deltaTime) {
	const Camera& camera = p_player->getCamera();
	switch (direction) {
	case FORWARD:
		m_horizontalDir += toHorizontal(camera.getFront());
		break;
	case BACKWARD:
		m_horizontalDir -= toHorizontal(camera.getFront());
		break;
	case RIGHT:
		m_horizontalDir += toHorizontal(camera.getRight());
		break;
	case LEFT:
		m_horizontalDir -= toHorizontal(camera.getRight());
		break;
	case UP:
		if (p_player->getGameMode() == GameMode::SURVIVAL) {
			if (m_onTheGround) {
				if (m_inWater)
					m_verticalSpeed = WATER_JUMP_SPEED;
				else
					m_verticalSpeed = JUMP_SPEED;
			} else {
				if (m_inWater)
					m_verticalSpeed += WATER_SWIM_UP_ACCELERATION * deltaTime;
			}
		} else {
			m_verticalDir += Camera::WORLDUP;
		}
		break;
	case DOWN:
		m_verticalDir -= Camera::WORLDUP;
		break;
	}
}

void Movement::update(float deltaTime) {
	if (p_player->getGameMode() == GameMode::SURVIVAL) {
		if (m_inWater) {
			m_verticalSpeed -= WATER_GRAVITY * deltaTime;
			m_verticalSpeed *= std::pow(WATER_VERTICAL_DRAG, deltaTime);
		} else {
			m_verticalSpeed -= GRAVITY * deltaTime;
		}
	}
}

vec3 Movement::getVelocityAndReset() {
	GameMode gameMode = p_player->getGameMode();
	float horizontal_speed = gameMode == GameMode::SURVIVAL ? WALK_HORIZONTAL_SPEED : FLY_HORIZONTAL_SPEED;
	float sprint_multiplier = gameMode == GameMode::SURVIVAL ? WALK_SPRINT_MULTIPLIER : FLY_SPRINT_MULTIPLIER;
	vec3 horizontal_move{};
	if (m_horizontalDir != vec3())
		horizontal_move = normalize(m_horizontalDir) * horizontal_speed;
	if (m_sprinting)
		horizontal_move *= sprint_multiplier;
	if (gameMode == GameMode::SURVIVAL && m_inWater)
		horizontal_move *= WATER_HORIZONTAL_MULTIPLIER;
	vec3 vertical_move;
	if (gameMode == GameMode::SURVIVAL) {
		vertical_move = Camera::WORLDUP * m_verticalSpeed;
	} else {
		m_verticalSpeed = 0; // Reset vertical speed
		vertical_move = m_verticalDir * FLY_VERTICAL_SPEED;
		if (m_sprinting)
			vertical_move *= sprint_multiplier;
	}
	m_horizontalDir = m_verticalDir = vec3();
	m_sprinting = false;
	return horizontal_move + vertical_move;
}

vec3 Movement::getMoveAndReset(float deltaTime) {
	if (p_player->getGameMode() == GameMode::SPECTATOR)
		return getVelocityAndReset() * deltaTime;
	else
		return getMoveWithCollisionsAndReset(deltaTime);
}

Box Movement::makeHitbox() const {
	vec3 hitbox_size{ PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_WIDTH };
	vec3 hitbox_pos = p_player->getPosition();
	hitbox_pos.x -= hitbox_size.x / 2;
	hitbox_pos.z -= hitbox_size.z / 2;
	hitbox_pos.y -= PLAYER_HEAD_HEIGHT;
	return { hitbox_pos, hitbox_size };
}

vec3 Movement::getMoveWithCollisionsAndReset(float deltaTime) {
	vec3 velocity = getVelocityAndReset();

	Box hitbox = makeHitbox();
	std::vector<ivec3> blocks = getBroadphaseBlocks(hitbox, velocity * deltaTime, [](Block block) {
		return ResManager::blockDatas().get(block.id).isObstacle();
	});
	std::vector<Box> bs;
	for (ivec3 block : blocks) {
		bs.push_back({ block, {1, 1, 1} });
	}
	vec3 shift = repeatedSweptAABB(hitbox, velocity, bs, deltaTime);

	if (p_player->getGameMode() == GameMode::SURVIVAL) {
		// Vertical motion is blocked when the player moved less far vertically than its velocity asked
		// for. Comparing against the requested displacement (rather than testing |shift.y| ~ 0) is robust.
		bool verticalBlocked = velocity.y != 0.f && std::abs(shift.y - velocity.y * deltaTime) > 1e-6f;
		// Grounded only when blocked while moving down
		m_onTheGround = velocity.y < 0.f && verticalBlocked;
		// Cancel vertical speed on landing or on hitting a ceiling
		if (verticalBlocked)
			m_verticalSpeed = 0.f;
		m_inWater = isInWater();
	}

	// Push the player out of any block it ends up embedded in (placed/generated inside it, etc.)
	shift += getUnstuckShift(deltaTime);

	return shift;
}

vec3 Movement::getUnstuckShift(float deltaTime) const {
	Box hitbox = makeHitbox();
	std::vector<ivec3> blocks = getBroadphaseBlocks(hitbox, vec3(), [](Block block) {
		return ResManager::blockDatas().get(block.id).isObstacle();
	});

	// Only treat a block as penetrating past this depth, so merely touching a floor or wall
	// (the usual case while standing or walking) never produces a push
	const float epsilon = 1e-4f;

	// Push out of each overlapping block along its own axis of least penetration. Resolving each
	// block on its shallowest axis lets a corner escape diagonally instead of oscillating between
	// the two perpendicular walls.
	vec3 push{};
	for (ivec3 bp : blocks) {
		vec3 overlap, dir;
		bool penetrating = true;
		for (int i = 0; i < 3; ++i) {
			float hmin = hitbox.pos[i], hmax = hmin + hitbox.size[i];
			float bmin = float(bp[i]), bmax = bmin + 1.f;
			float penPos = bmax - hmin; // depth to exit towards +i
			float penNeg = hmax - bmin; // depth to exit towards -i
			overlap[i] = std::min(penPos, penNeg);
			dir[i] = penPos < penNeg ? 1.f : -1.f;
			if (overlap[i] <= epsilon)
				penetrating = false;
		}
		if (!penetrating)
			continue;

		int axis = 0;
		for (int i = 1; i < 3; ++i)
			if (overlap[i] < overlap[axis])
				axis = i;

		// Blocks sharing an axis don't stack: keep the deepest push needed on each axis
		float p = dir[axis] * overlap[axis];
		if (std::abs(p) > std::abs(push[axis]))
			push[axis] = p;
	}

	float dist = length(push);
	if (dist <= epsilon)
		return vec3(); // Not penetrating anything

	// Eject smoothly, capped per frame so the player is pushed out rather than teleported
	return push * (std::min(dist, PUSH_OUT_SPEED * deltaTime) / dist);
}

bool Movement::isInWater() const {
	return !getBroadphaseBlocks(makeHitbox(), vec3(), [](Block block) {
		return ResManager::blockDatas().get(block.id).getCategory() == BlockData::Category::WATER;
	}).empty();
}

bool Movement::intersectsBlock(ivec3 blockPos) const {
	return aabb_check(makeHitbox(), { blockPos, {1, 1, 1} });
}

std::vector<ivec3> Movement::getBroadphaseBlocks(const Box& hitbox, vec3 shift,
		std::function<bool(Block)> filter) const {

	std::vector<ivec3> blocks;
	Box b; ivec3 dirs;
	std::tie(b, dirs) = getBroadphaseBox(hitbox, shift);
	ivec3 pos;
	const ChunkMap& cm = p_player->getGame().getChunkMap();
	
	std::function<void(int)> rec = [&rec, &blocks, &b, &dirs, &pos, &cm, &filter](int axis) {
		if (axis == 3) {
			Block block = cm.getBlock(pos);
			if (filter(block))
				blocks.push_back(pos);
			return;
		}
		// Start on the side at the opposite of the hitbox
		float left = b.pos[axis], right = b.pos[axis] + b.size[axis];
		int& coord = pos[axis];
		if (dirs[axis] == 1) {
			for (coord = int(floor(right)); coord > left - 1; --coord) {
				rec(axis + 1);
			}
		} else {
			for (coord = int(floor(left)); coord < right; ++coord) {
				rec(axis + 1);
			}
		}
	};
	rec(0);
	return blocks;
}