#include "Commands.h"

Commands::Commands() {
	using namespace sf;
	if (Keyboard::isKeyPressed(Keyboard::Z))
		onPressedEvent(Command::FORWARD);
	if (Keyboard::isKeyPressed(Keyboard::S))
		onPressedEvent(Command::BACKWARD);
	if (Keyboard::isKeyPressed(Keyboard::Q))
		onPressedEvent(Command::LEFT);
	if (Keyboard::isKeyPressed(Keyboard::D))
		onPressedEvent(Command::RIGHT);
	if (Keyboard::isKeyPressed(Keyboard::Space))
		onPressedEvent(Command::UP);
	if (Keyboard::isKeyPressed(Keyboard::LShift))
		onPressedEvent(Command::DOWN);
}

Command Commands::findKey(sf::Keyboard::Key key) const {
	using namespace sf;
	switch (key) {
	case Keyboard::E: return Command::EXPLODE;
	case Keyboard::T: return Command::TELEPORT;
	default: return Command::UNKNOWN;
	}
}

Command Commands::findMouseButton(sf::Mouse::Button button) const {
	using namespace sf;
	switch (button) {
	case Mouse::Button::Left: return Command::BREAK;
	case Mouse::Button::Right: return Command::PLACE;
	case Mouse::Button::Middle: return Command::PICK;
	default: return Command::UNKNOWN;
	}
}

void Commands::onPressedEvent(sf::Keyboard::Key key) {
	onPressedEvent(findKey(key));
}

void Commands::onPressedEvent(sf::Mouse::Button button) {
	onPressedEvent(findMouseButton(button));
}

void Commands::onPressedEvent(Command command) {
	m_commands.insert(command);
}

bool Commands::isActive(Command command) const {
	return m_commands.find(command) != m_commands.end();
}

bool Commands::use(Command command) {
	auto it = m_commands.find(command);
	if (it == m_commands.end()) {
		return false;
	} else {
		m_commands.erase(it);
		return true;
	}
}
