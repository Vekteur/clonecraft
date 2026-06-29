#include "Commands.h"

Commands::Commands() {
	pollHeld();
}

void Commands::pollHeld() {
	using namespace sf;
	if (Keyboard::isKeyPressed(Keyboard::Scan::W)) activate(Command::FORWARD);
	if (Keyboard::isKeyPressed(Keyboard::Scan::S)) activate(Command::BACKWARD);
	if (Keyboard::isKeyPressed(Keyboard::Scan::A)) activate(Command::LEFT);
	if (Keyboard::isKeyPressed(Keyboard::Scan::D)) activate(Command::RIGHT);
	if (Keyboard::isKeyPressed(Keyboard::Space)) activate(Command::UP);
	if (Keyboard::isKeyPressed(Keyboard::LShift)) activate(Command::DOWN);
	if (Keyboard::isKeyPressed(Keyboard::LControl)) activate(Command::SPRINT);
	if (Mouse::isButtonPressed(Mouse::Button::Left)) activate(Command::BREAK);
	if (Mouse::isButtonPressed(Mouse::Button::Right)) activate(Command::PLACE);
	if (Mouse::isButtonPressed(Mouse::Button::Middle)) activate(Command::PICK);
}

void Commands::onKeyPressed(sf::Keyboard::Key key) {
	activate(findKey(key));
}

Command Commands::findKey(sf::Keyboard::Key key) {
	using namespace sf;
	switch (key) {
	case Keyboard::E:
		return Keyboard::isKeyPressed(Keyboard::LAlt) ? Command::HUGE_EXPLOSION : Command::EXPLOSION;
	case Keyboard::B:
		return Keyboard::isKeyPressed(Keyboard::LAlt) ? Command::HUGE_BRUSH : Command::BRUSH;
	case Keyboard::P: return Command::PLACE_BELOW;
	case Keyboard::T: return Command::TELEPORT;
	case Keyboard::G: return Command::NEXT_GAMEMODE;
	default: return Command::UNKNOWN;
	}
}

void Commands::activate(Command command) {
	if (command != Command::UNKNOWN)
		m_commands.insert(command);
}

bool Commands::isActive(Command command) const {
	return m_commands.find(command) != m_commands.end();
}
