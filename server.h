#pragma once

class Zone;
class Luawrapper;
class Player;

#include "script.h"
#include "uuid.h"

#include <map>
#include <thread>
#include <mutex>

#define MAX_SOCKET_QUEUE 8

// TODO : Add mutexes around every atomic non-cascading operations.

class Server {
public:
	Server();
	~Server();
	void _open(unsigned short port);
	void _close();
	bool isOpen();
	unsigned short getPort();
	void addZone(std::string id, class Zone * zone); // Automatically done by new Zone().
	class Zone * getZone(std::string id);
	void delZone(std::string id);
	void addPlayer(class Player * player); // Automatically called by new Player()
	class Player * getPlayer(int id); // May return nullptr.
	void delPlayer(int id);
	void remPlayer(int id);

	/* Player actions */
// TODO: Stronger type: trigger.
	void addAction(const Script& script, std::string trigger);
	const Script& getAction(std::string trigger); // May return Script::noValue.
	void delAction(std::string trigger);
	void doAction(std::string trigger, class Player& player, std::string arg = "");

	/* Timers */
	Uuid addTimer(unsigned int duration, const Script& script);
	void delTimer(Uuid id);
	void triggerTimer(Uuid id);
	unsigned int getTimerRemaining(Uuid id); // 0 is not-found.
	void setTimerRemaining(Uuid id, unsigned int remaining);

	class Luawrapper * getLua();
	void waitForTerminaison();

private:
	int connexion_fd;
	unsigned short port;
	std::map<std::string, class Zone *> zones;
	std::map<int, class Player *> players;
	std::map<std::string, Script> actions;

	/* Timers */
	struct Timer { unsigned int remaining; Script script; };
	std::map<Uuid,struct Timer> timers;
	std::thread timersThread;
	// std::mutex timersLock; // TODO
	void timersLoop();

	std::thread * acceptThread;
	class Luawrapper * luawrapper;
	std::mutex terminaisonLock;

	void acceptLoop();
};
