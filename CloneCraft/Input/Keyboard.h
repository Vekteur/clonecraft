#pragma once

class Keyboard
{
public:
	Keyboard() = delete;
	
	static void setKey(int key, bool enable);
	static bool isKeyPressed(int key);

	static bool s_keys[1024];
};

