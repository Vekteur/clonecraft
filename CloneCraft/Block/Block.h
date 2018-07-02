#pragma once

#include "json.hpp"
using json = nlohmann::json;

class Block {
public:

	enum Direction {
		UP,
		DOWN,
		NORTH,
		SOUTH,
		EAST,
		WEST,
		SIZE
	};

	Block(const json& j);
	~Block();

	bool isOpaque();
	bool isObstacle();
	int getResistance();
	int getTexture(Direction dir);

private:
	int m_textures[SIZE];
	bool m_opaque;
	bool m_obstacle;
	int m_resistance;
};
