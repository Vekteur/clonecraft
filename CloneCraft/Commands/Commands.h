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
	EXPLODE,
	TELEPORT,
	UNKNOWN,
	SIZE
};

class Commands {
public:
	Commands();
	Command findKey(sf::Keyboard::Key key) const;
	Command findMouseButton(sf::Mouse::Button button) const;
	void onPressedEvent(sf::Keyboard::Key key);
	void onPressedEvent(sf::Mouse::Button button);
	void onPressedEvent(Command command);
	bool isActive(Command command) const;
	bool use(Command command);

	std::set<Command> m_commands;
};

