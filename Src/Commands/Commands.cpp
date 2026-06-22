#include "Commands.h"

Commands::Commands() {
	using namespace sf;
	if (Keyboard::isKeyPressed(Keyboard::Scan::W))
		onPressedEvent(Command::FORWARD);
	if (Keyboard::isKeyPressed(Keyboard::Scan::S))
		onPressedEvent(Command::BACKWARD);
	if (Keyboard::isKeyPressed(Keyboard::Scan::A))
		onPressedEvent(Command::LEFT);
	if (Keyboard::isKeyPressed(Keyboard::Scan::D))
		onPressedEvent(Command::RIGHT);
	if (Keyboard::isKeyPressed(Keyboard::Space))
		onPressedEvent(Command::UP);
	if (Keyboard::isKeyPressed(Keyboard::LShift))
		onPressedEvent(Command::DOWN);
	if (Keyboard::isKeyPressed(Keyboard::LControl))
		onPressedEvent(Command::SPRINT);
}

Command Commands::findKey(sf::Keyboard::Key key) const {
	using namespace sf;
	switch (key) {
	case Keyboard::E:
		if (Keyboard::isKeyPressed(Keyboard::LAlt)) return Command::HUGE_EXPLOSION;
		else return Command::EXPLOSION;
	case Keyboard::B:
		if (Keyboard::isKeyPressed(Keyboard::LAlt)) return Command::HUGE_BRUSH;
		else return Command::BRUSH;
	case Keyboard::P: return Command::PLACE_BELOW;
	case Keyboard::T: return Command::TELEPORT;
	case Keyboard::G: return Command::NEXT_GAMEMODE;
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
