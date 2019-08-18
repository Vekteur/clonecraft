#pragma once

#include "BlockID.h"

struct Block {
	BlockID id;
	char data;

	Block(BlockID id = BlockID::AIR, char data = 0) : id{ id }, data{ data } {}
};