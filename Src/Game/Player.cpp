#include "Player.h"

#include <Game/Game.h>
#include <Maths/LineBlockFinder.h>

const float Player::TARGET_DISTANCE{ static_cast<float>(ChunkMap::VIEW_DISTANCE * Const::SECTION_SIDE) };

Player::Player(Game* game)
	: game{ game }, m_camera{ vec3{ 0.0f, 80.0f, 0.0f } }, m_movement{ this } { }

void Player::processMouseClick(sf::Time dt, Commands& commands) {
	if (commands.isActive(Command::PICK) && targetPos.has_value()) {
		pickedBlock = game->getChunkMap().getBlock(targetPos.value());
	}
	breakAccumulator += dt;
	if (commands.isActive(Command::BREAK) && breakAccumulator >= sf::seconds(0.1f) && targetPos.has_value()
		&& game->canReloadBlocks()) {

		game->getChunkMap().setBlock(targetPos.value(), +BlockID::AIR);
		game->reloadBlocks({ targetPos.value() });
		breakAccumulator = sf::seconds(0.f);
	}
	placeAccumulator += dt;
	if (commands.isActive(Command::PLACE) && placeAccumulator >= sf::seconds(0.1f) && placePos.has_value() &&
		pickedBlock.has_value() && game->canReloadBlocks()) {

		game->getChunkMap().setBlock(placePos.value(), pickedBlock.value());
		game->reloadBlocks({ placePos.value() });
		placeAccumulator = sf::seconds(0.f);
	}
}

void Player::processMouseMove(sf::Time dt) {
	sf::Vector2i mousePosTemp{ sf::Mouse::getPosition(game->getWindow()) };
	ivec2 mousePosition{ mousePosTemp.x, mousePosTemp.y };
	ivec2 windowCenter{ game->getWindow().getCenter() };
	ivec2 mouseOffset{ mousePosition - windowCenter };
	m_camera.processMouse(mouseOffset);
}

void Player::processMouseWheel(sf::Time dt, GLfloat delta) {
	m_camera.processMouseScroll(delta);
}

void Player::update(sf::Time dt) {
	m_movement.update(dt.asSeconds());
	m_camera.move(m_movement.getMoveAndReset(dt.asSeconds()));
	m_camera.update({ game->getWindow().size() });

	LineBlockFinder lineBlockFinder{ m_camera.getPosition(), m_camera.getFront() };
	placePos = std::nullopt;
	targetPos = std::nullopt;
	while (lineBlockFinder.getDistance() <= TARGET_DISTANCE) {
		ivec3 iterPos = lineBlockFinder.next();
		Block block = game->getChunkMap().getBlock(iterPos);
		BlockData::Category category = ResManager::blockDatas().get(block.id).getCategory();
		if (category != BlockData::AIR && category != BlockData::WATER) {
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

void Player::move(Movement::Direction direction, sf::Time dt) {
	m_movement.move(direction, dt.asSeconds());
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

Camera& Player::getCamera() {
	return m_camera;
}

const Camera& Player::getCamera() const {
	return m_camera;
}

std::optional<ivec3> Player::getTarget() {
	return targetPos;
}