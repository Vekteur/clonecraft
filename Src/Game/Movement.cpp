#include "Movement.h"

#include <Game/Player.h>
#include <Game/Game.h>

const float Movement::HORIZONTAL_SPEED{ 30.f };
const float Movement::VERTICAL_SPEED{ 20.f };
const float Movement::PLAYER_WIDTH{ 0.6f };
const float Movement::PLAYER_HEIGHT{ 1.8f };
const float Movement::PLAYER_HEAD_HEIGHT{ 1.65f };

Movement::Movement(const Player* player)
	: p_player(player)
{ }

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
			if (m_inWater)
				m_verticalSpeed += 20.f * deltaTime;
			else if (m_onTheGround)
				m_verticalSpeed = 10.f;
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
		if (m_inWater)
			m_verticalSpeed -= 10.f * deltaTime;
		else
			m_verticalSpeed -= 30.f * deltaTime;
	}
}

vec3 Movement::getVelocityAndReset() {
	vec3 horizontal_move{};
	if (m_horizontalDir != vec3())
		horizontal_move = normalize(m_horizontalDir) * HORIZONTAL_SPEED;
	if (p_player->getGameMode() == GameMode::SURVIVAL && m_inWater)
		horizontal_move *= 0.6f;
	vec3 vertical_move;
	if (p_player->getGameMode() == GameMode::SURVIVAL)
		vertical_move = Camera::WORLDUP * m_verticalSpeed;
	else
		vertical_move = m_verticalDir * VERTICAL_SPEED;
	m_horizontalDir = m_verticalDir = vec3();
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
		// If the player is falling but does not move, it is against the ground
		m_onTheGround = velocity.y != 0.f && std::abs(shift.y) < 2e-6;
		if (m_onTheGround)
			m_verticalSpeed = 0.f;
		m_inWater = isInWater();
	}
	
	return shift;
}

bool Movement::isInWater() const {
	return !getBroadphaseBlocks(makeHitbox(), vec3(), [](Block block) {
		return ResManager::blockDatas().get(block.id).getCategory() == BlockData::Category::WATER;
	}).empty();
}

std::vector<ivec3> Movement::getBroadphaseBlocks(const Box& hitbox, vec3 shift,
		std::function<bool(Block)> filter) const {

	std::vector<ivec3> blocks;
	Box b; ivec3 dirs;
	std::tie(b, dirs) = getBroadphaseBox(hitbox, shift);
	ivec3 pos;
	const ChunkMap& cm = p_player->game->getChunkMap();
	
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