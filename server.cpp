#include "server.h"

#include "player.h"
#include "zone.h"
#include "luawrapper.h"
#include "log.h"

#include <thread>

#ifdef __linux__
#include <unistd.h> // close()
#include <sys/socket.h> // socket(), bind(), listen()
#include <netinet/in.h> // IPv4, IPv6
#include <arpa/inet.h> // inet_ntoa()

// #include <pthread.h>

#elif defined _WIN32
#endif

/* Static */

/* Private */

void Server::acceptLoop() {
	struct sockaddr_in remote_addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int fd;

	while(this->isOpen()) {
		fd = accept(connexion_fd, (struct sockaddr*) &remote_addr, &addr_len);
		if(fd == -1) {
			warning("Accept new connexion failed");
		} else {
			info(
					"Got connexion from "
					+ std::string(inet_ntoa(remote_addr.sin_addr))
					+ " port "
					+ std::to_string(ntohs(remote_addr.sin_port))
					+ " on socket #"
					+ std::to_string(fd)
					);
			class Player * player = new Player(fd, Name{}, Aspect{});
			this->addPlayer(player);
			this->luawrapper->spawnScript(player);
			if(player->getZone() == nullptr) {
				warning("Spawn script didn't call spawn().");
				this->delPlayer(player->getId());
			}
		}

	}

	this->acceptThread = nullptr;
}

void Server::timersLoop() {
	while(true) {
		usleep(1000000);
		// timersLock.lock();
		for(auto it = this->timers.begin(); it != this->timers.end(); ) {
			it->second.remaining--;
			if(it->second.remaining == 0) {
				it->second.script.execute(*luawrapper);
				it = this->timers.erase(it);
			} else {
				it++;
			}
		}
		// timersLock.unlock();
	}
}

/* Public */

Server::Server() :
	// 0 is a valid file descriptor, but not a valid socket file descriptor.
	connexion_fd(0),
	port(0),
	timersThread(std::thread(&Server::timersLoop, this)),
	acceptThread(nullptr),
	luawrapper(new Luawrapper(this))
{
	this->terminaisonLock.lock();
	// Main loop thread starting is done at _open(), not at constructor.
}

Server::~Server() {
	// Main loop thread stopping is done at _close(), not at destructor.
	if(this->isOpen()) {
		this->_close();
	}

	for(std::pair<std::string, Zone*> it : this->zones) {
		delete(it.second);
	}

	this->terminaisonLock.unlock();
}

void Server::_open(unsigned short port) {
	if(this->connexion_fd != 0) {
		this->_close();
	}
	this->port = port;

	// Open connexion.
#ifdef __linux__
	int sockfd;
	struct sockaddr_in addr;
	int yes=1;

	if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		warning("Unable to create socket");
		return;
	}
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		warning("Unable to set socket option : SO_REUSEADDR");
		return;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = 0;
	// memset(&(host_addr.sin_zero), '\0', 8); // FIXME: is this really useless ?
	if(bind(sockfd,(struct sockaddr *) &addr, sizeof(struct sockaddr)) == -1) {
		warning("Unable to bind socket to port");
		return;
	}
	if(listen(sockfd, MAX_SOCKET_QUEUE) == -1) {
		warning("Unable to listen on socket");
		return;
	}

	this->connexion_fd = sockfd;

	// Make asynchronous.
	// fcntl(servsock, F_SETFL, fcntl(servsock, F_GETFL) | O_ASYNC);

	// Ask the kernel to send us SIGIO on new connexion.
	// fcntl(servsock, F_SETOWN, getpid());

#elif defined _WIN32
#endif

	info("Server opened.");
	// TODO : IPv4/IPv6 connexions.

	// Start connexion accepting thread.
	this->acceptThread = new std::thread(&Server::acceptLoop, this);
}

void Server::_close() {
#ifdef __linux__
	close(this->connexion_fd);
#elif defined _WIN32
#endif
	this->connexion_fd = 0;
	this->port = 0;

	if(this->acceptThread != nullptr) {
		this->acceptThread->detach();
		delete(this->acceptThread);
		// this->acceptThread->join();
		this->acceptThread = nullptr;
	}

	info("Server closed.");
}

bool Server::isOpen() {
	return(this->connexion_fd != 0);
}

unsigned short Server::getPort() {
	if(this->isOpen()) {
		return(this->port);
	} else {
		return(0);
	}
}

// Automatically done by new Zone().
void Server::addZone(std::string id, class Zone * zone) {
	if(this->getZone(id) != nullptr) {
		this->delZone(id);
		info("Zone '"+id+"' replaced.");
	}
	this->zones[id] = zone;
}

class Zone * Server::getZone(std::string id) {
	return(this->zones[id]);
}

void Server::delZone(std::string id) {
	if(this->zones[id] != nullptr) {
		delete(this->zones[id]);
		this->zones.erase(id);
	}
}

void Server::addPlayer(class Player * player) {
	int id = player->getId();
	if(this->players[id] != nullptr) {
		warning("Player '"+std::to_string(id)+"' replaced.");
		delete(this->players[id]);
	}
	this->players[id] = player;
}

class Player * Server::getPlayer(int id) {
	class Player * player = this->players[id];
	if(player == nullptr) {
		return(nullptr);
	} else {
		return(player);
	}
}

void Server::delPlayer(int id) {
	class Player * player = this->players[id];
	if(player == nullptr) {
		info("Player '"+std::to_string(id)+
				"' can't be deleted: doesn't exist.");
	} else {
		delete(player);
		this->remPlayer(id);
	}
}

void Server::remPlayer(int id) {
	this->players.erase(id);
}

void Server::addAction(const Script& script, std::string trigger) {
	if(this->actions.count(trigger) > 0) {
		warning("Action '"+trigger+"' replaced.");
	}
	this->actions[trigger] = script;
}

const Script& Server::getAction(std::string trigger) {
	try {
		return(this->actions.at(trigger));
	} catch (const std::out_of_range& oor) {
		return(Script::noValue);
	}
}

void Server::delAction(std::string trigger) {
	if(this->actions.count(trigger) == 0) {
		info("Action '"+trigger+"' can't be deleted: doesn't exist.");
	} else {
		this->actions.erase(trigger);
	}
}

void Server::doAction(std::string trigger, class Player& player, std::string arg) {
	try {
		this->actions.at(trigger).execute(*(this->luawrapper), &player, arg);
	} catch (const std::out_of_range& oor) {
		info("Action '"+trigger+"' doesn't exist.");
	}
}

Uuid Server::addTimer(unsigned int duration, const Script& script) {
	Uuid id {};
	this->timers.emplace(id, Timer{duration, script});
	return(id);
}

void Server::delTimer(Uuid id) {
	this->timers.erase(id);
}

void Server::triggerTimer(Uuid id) {
	auto it = this->timers.find(id);
	if(it == this->timers.end()) {
		warning("Cannot trigger timer: id not found.");
		return;
	}
	it->second.script.execute(*luawrapper);
	this->timers.erase(it);
}

unsigned int Server::getTimerRemaining(Uuid id) {
	auto it = this->timers.find(id);
	if(it == this->timers.end()) {
		return(0);
	}
	return(it->second.remaining);
}

void Server::setTimerRemaining(Uuid id, unsigned int remaining) {
	if(remaining == 0) {
		this->triggerTimer(id);
		return;
	}
	auto it = this->timers.find(id);
	if(it == this->timers.end()) {
		warning("Cannot set timer's remaining time: id not found.");
		return;
	}
	it->second.remaining = remaining;
}

class Luawrapper * Server::getLua() {
	return(this->luawrapper);
}

void Server::waitForTerminaison() {
	this->terminaisonLock.lock();
}
