#pragma once

#include "BlockID.h"

struct Light {
	// Sky light (high 4 bits) and block light (low 4 bits).
	unsigned char light;

	unsigned char sky() const { return light >> 4; }
	unsigned char block() const { return light & 0x0F; }
	void setSky(unsigned char level) { light = (light & 0x0F) | (level << 4); }
	void setBlock(unsigned char level) { light = (light & 0xF0) | (level & 0x0F); }
};

struct Block {
	BlockID id;
	char data;
	Light light;

	Block(BlockID id = BlockID::AIR, char data = 0, Light light = { 0xF0 })
		: id{ id }, data{ data }, light{ light } {}
};
