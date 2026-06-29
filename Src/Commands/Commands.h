#pragma once

#include <set>

#include <SFML/Window.hpp>

enum class Command {
	BREAK,
	PLACE,
	PICK,
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN,
	SPRINT,
	EXPLOSION,
	HUGE_EXPLOSION,
	BRUSH,
	HUGE_BRUSH,
	PLACE_BELOW,
	TELEPORT,
	NEXT_GAMEMODE,
	UNKNOWN,
	SIZE
};

class Commands {
public:
	Commands();
	void onKeyPressed(sf::Keyboard::Key key);
	bool isActive(Command command) const;

private:
	void pollHeld();
	void activate(Command command);
	static Command findKey(sf::Keyboard::Key key);

	std::set<Command> m_commands;
};
