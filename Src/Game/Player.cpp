#include "Player.h"

#include "Maths/Converter.h"

#include <Game/Game.h>
#include <Maths/LineBlockFinder.h>

const vec3 Player::INITIAL_POSITION{ vec3{ 0.f, 80.f, 0.f } };
const float Player::DEFAULT_TARGET_DISTANCE{ 6. };
const float Player::CREATIVE_TARGET_DISTANCE{ static_cast<float>(ChunkMap::VIEW_DISTANCE * Const::SECTION_SIDE) };
const sf::Time Player::REPEAT_DELAY{ sf::seconds(0.2f) };

Player::Player(Game* game)
	: game{ game }, m_camera{ INITIAL_POSITION }, m_movement{ this } { }

void Player::processMouseClick(sf::Time dt, Commands& commands) {
	if (commands.isActive(Command::PICK) && targetPos.has_value())
		pickedBlock = game->getChunkMap().getBlock(targetPos.value());

	bool breaking = commands.isActive(Command::BREAK) && targetPos.has_value();
	if (tryRepeat(breakAccumulator, dt, breaking))
		game->getChunkMap().setBlock(targetPos.value(), +BlockID::AIR);

	bool placing = commands.isActive(Command::PLACE) && placePos.has_value() &&
		pickedBlock.has_value() && !intersectsBlock(placePos.value());
	if (tryRepeat(placeAccumulator, dt, placing))
		game->getChunkMap().setBlock(placePos.value(), pickedBlock.value());
}

// Advances a repeat timer. Returns true when the action should fire: right away on
// the first active frame, then once every REPEAT_DELAY while it stays held.
bool Player::tryRepeat(sf::Time& accumulator, sf::Time dt, bool active) {
	if (!active) {
		// Stay primed so the next click acts without waiting.
		accumulator = REPEAT_DELAY;
		return false;
	}
	accumulator += dt;
	if (accumulator < REPEAT_DELAY)
		return false;
	accumulator = sf::seconds(0.f);
	return true;
}

void Player::processMouseMove(sf::Time) {
	sf::Vector2i mousePosTemp{ sf::Mouse::getPosition(game->getWindow()) };
	ivec2 mousePosition{ mousePosTemp.x, mousePosTemp.y };
	ivec2 windowCenter{ game->getWindow().getCenter() };
	ivec2 mouseOffset{ mousePosition - windowCenter };
	m_camera.processMouse(mouseOffset);
}

void Player::processMouseWheel(sf::Time, GLfloat delta) {
	m_camera.processMouseScroll(delta);
}

void Player::update(sf::Time dt) {
	m_movement.update(dt.asSeconds());
	m_camera.move(m_movement.getMoveAndReset(dt.asSeconds()));
	m_camera.update({ game->getWindow().size() });

	LineBlockFinder lineBlockFinder{ m_camera.getPosition(), m_camera.getFront() };
	placePos = std::nullopt;
	targetPos = std::nullopt;
	float targetDistance = m_gameMode == GameMode::CREATIVE ? CREATIVE_TARGET_DISTANCE : DEFAULT_TARGET_DISTANCE;
	while (lineBlockFinder.getDistance() <= targetDistance) {
		ivec3 iterPos = lineBlockFinder.next();
		Block block = game->getChunkMap().getBlock(iterPos);
		if (ResManager::blockDatas().get(block.id).isObstacle()) {
			targetPos = iterPos;
			break;
		}
		placePos = iterPos;
	}
	if (targetPos == std::nullopt)
		placePos = std::nullopt;
}

void Player::teleport() {
	if (targetPos.has_value()) {
		vec3 pos = targetPos.value();
		pos += vec3(0.5, 1 + Movement::PLAYER_HEAD_HEIGHT, 0.5);
		m_camera.setPosition(pos);
	}
}

void Player::placeBlockBelow() {
	if (pickedBlock.has_value()) {
		vec3 pos = getPosition();
		pos.y -= 1.f + Movement::PLAYER_HEAD_HEIGHT;
		game->getChunkMap().setBlock(Converter::globalPosToBlock(pos), pickedBlock.value());
	}
}

void Player::move(Movement::Direction direction, sf::Time dt) {
	m_movement.move(direction, dt.asSeconds());
}

void Player::setSprinting(bool sprinting) {
	m_movement.setSprinting(sprinting);
}

vec3 Player::getPosition() const {
	return m_camera.getPosition();
}

void Player::nextGameMode() {
	int next_int = (static_cast<int>(m_gameMode) + 1) % static_cast<int>(GameMode::SIZE);
	m_gameMode = static_cast<GameMode>(next_int);
}

void Player::setGameMode(GameMode gameMode) {
	m_gameMode = gameMode;
}

GameMode Player::getGameMode() const {
	return m_gameMode;
}

Game& Player::getGame() {
	return *game;
}

const Game& Player::getGame() const {
	return *game;
}

Camera& Player::getCamera() {
	return m_camera;
}

const Camera& Player::getCamera() const {
	return m_camera;
}

std::optional<ivec3> Player::getTarget() const {
	return targetPos;
}

std::optional<Block> Player::getPickedBlock() const {
	return pickedBlock;
}

bool Player::isInWater() const {
	return m_movement.isInWater();
}

bool Player::intersectsBlock(ivec3 blockPos) const {
	return m_movement.intersectsBlock(blockPos);
}
