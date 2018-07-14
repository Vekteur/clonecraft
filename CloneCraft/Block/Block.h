#pragma once

#include "ID.h"

struct Block
{
	ID id;
	char data;

	Block(ID id = ID::AIR, char data = 0) : id{ id }, data{ data } {}
};