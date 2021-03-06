#pragma once

#include "aspect.h"

#include <thread>

class Character;

class Player {
public:
	Player(int fd, class Character* character);
	~Player();

	void check_action();
	bool delme();

	/* Send messages to client */
	void message(std::string message);
	void updateCharacter(class Character * character);
	void updateCharacterExit(class Character * character);
	void updateFloor();
	void updateFloor(unsigned int x, unsigned int y, const Aspect& aspect);
	void updateGauge(
		std::string name,
		unsigned int val,
		unsigned int max,
		const Aspect& full,
		const Aspect& empty);
	void updateNoGauge(std::string name);
/* XXX //
	void updateInventory(unsigned long int id, Aspect aspect);
	void updateNoInventory(unsigned long int id);
	void addPickupList(unsigned long int id, Aspect aspect);
	void remPickupList(unsigned long int id);
// XXX */
	void follow(class Character * character);
	void hint(Aspect aspect, std::string hint);
private:
	int fd;
	class Character* character;
	bool _delme = false;

	void send(std::string message);
	std::string receive();
	void parse(std::string message);
};
