#include "Block.h"

Block::Block(const json& j)
{
	m_opaque = j["opaque"];
	m_obstacle = j["obstacle"];
	m_resistance = j["resistance"];
	
	///TODO textures
}


Block::~Block()
{
}

bool Block::isOpaque()
{
	return m_opaque;
}

bool Block::isObstacle()
{
	return m_obstacle;
}

int Block::getResistance()
{
	return m_resistance;
}

int Block::getTexture(Direction dir)
{
	return m_textures[dir];
}

