#pragma once

class Place;
class Server;
class Tile;
class Place;
class Player;

#include "error.h"
#include "aspect.h"
#include "name.h"

#include <string>
#include <vector>
#include <list>

class Zone : public Named {
public:
	Zone(
		class Server * server,
		std::string id,
		const Name& name,
		unsigned int width,
		unsigned int height,
		class Tile * baseTile
	);
	~Zone();
	class Server * getServer();
	std::string getId();
	unsigned int getWidth();
	unsigned int getHeight();
	bool isPlaceValid(int x, int y);
	class Tile * getTile(int x, int y); // May return nullptr.
	void setTile(int x, int y, class Tile * tile); // And broadcast it.
	std::string * getLandOn(int x, int y); // May return nullptr.
	void setLandOn(int x, int y, std::string script);
	void resetLandOn(int x, int y);
	void event(std::string message); // Broadcast a message to all players.

	std::string getTag(int x, int y, std::string id);
	void setTag(int x, int y, std::string id, std::string value);
	void delTag(int x, int y, std::string id);

	/* Called by Player only */

	bool canLandPlayer(class Player * player, int x, int y);

	// Add the player to the zone,
	// send him the floor and broadcast its position.
	void enterPlayer(class Player * player, int x, int y);

	// Remove the player from the zone and broadcast it.
	void exitPlayer(class Player * player);

	// Broadcast the new position and aspect of the player.
	void updatePlayer(class Player * player);

private:
	class Server * server;
	std::string id;
	unsigned int width;
	unsigned int height;
	std::vector<class Place> places;
	std::list<int> players;

	class Player * getPlayer(int id); // Auto remove if invalid.
	class Place * getPlace(int x, int y);
	void updateTile(int x, int y);
};
