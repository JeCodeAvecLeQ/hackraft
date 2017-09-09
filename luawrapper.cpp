#include "luawrapper.h"

#include "log.h"
#include "gauge.h"
#include "inventory.h"
#include "name.h"
#include "place.h"
#include "player.h"
#include "server.h"
#include "zone.h"

#include <cstdlib> // rand()

class Server * Luawrapper::server = nullptr;

void lua_arg_error(std::string msg) {
	warning("Lua must call '"+msg+"'.");
}

int l_c_rand(lua_State * lua) {
	if(not lua_isnumber(lua, 1)) {
		lua_arg_error("c_rand(max)");
		lua_pushnil(lua);
	} else {
		int max = lua_tointeger(lua, 1);
		int x = rand()%max+1;
		lua_pushinteger(lua, x);
	}
	return(1);
}

/* Output */

int l_setverbose(lua_State * lua) {
	setVerbose();
	return(0);
}

int l_setnoverbose(lua_State * lua) {
	setNoVerbose();
	return(0);
}

int l_isverbose(lua_State * lua) {
	lua_pushboolean(lua, isVerbose());
	return(1);
}

int l_info(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("info(message)");
	} else {
		std::string message = lua_tostring(lua, 1);
		info(message);
	}
	return(0);
}

int l_warning(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("warning(message)");
	} else {
		std::string message = lua_tostring(lua, 1);
		warning(message);
	}
	return(0);
}

int l_fatal(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("fatal(message)");
	} else {
		std::string message = lua_tostring(lua, 1);
		fatal(message);
	}
	return(0);
}

/* Server */

int l_halt(lua_State * lua) {
	delete(Luawrapper::server);
	return(0);
}

int l_open(lua_State * lua) {
	if(not lua_isnumber(lua, 1)) {
		lua_arg_error("open(port)");
	} else {
		unsigned short int port = lua_tointeger(lua, 1);
		Luawrapper::server->_open(port);
	}
	return(0);
}

int l_close(lua_State * lua) {
	Luawrapper::server->_close();
	return(0);
}

int l_is_open(lua_State * lua) {
	lua_pushboolean(lua, Luawrapper::server->isOpen());
	return(1);
}

int l_get_port(lua_State * lua) {
	lua_pushinteger(lua, Luawrapper::server->getPort());
	return(1);
}

int l_delete_zone(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("delete_zone(zone_id)");
	} else {
		std::string id = lua_tostring(lua, 1);
		Luawrapper::server->delZone(id);
	}
	return(0);
}

/* Actions */

int l_add_action(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("add_action(trigger, script)");
	} else {
		std::string trigger = lua_tostring(lua, 1);
		Script script = Script { lua_tostring(lua, 2) };
		Luawrapper::server->addAction(script, trigger);
	}
	return(0);
}

int l_get_action(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("get_action(trigger)");
		lua_pushnil(lua);
	} else {
		std::string trigger = lua_tostring(lua, 1);
		const Script& script = Luawrapper::server->getAction(trigger);
		if(script != Script::noValue) {
			lua_pushstring(lua, script.toString().c_str());
		} else {
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_delete_action(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("delete_action(trigger)");
	} else {
		std::string trigger = lua_tostring(lua, 1);
		Luawrapper::server->delAction(trigger);
	}
	return(0);
}

// TODO : list_scripts();

int l_register_aspect(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isinteger(lua, 2)) {
		lua_arg_error("register_aspect(string, int [, passable])");
	} else {
		Aspect aspect { lua_tostring(lua, 1) };
		int entry = lua_tointeger(lua, 2);
		if(lua_isboolean(lua, 3)) {
			bool default_passable = lua_toboolean(lua, 3);
			Aspect::registerAspect(aspect, entry, default_passable);
		} else {
			Aspect::registerAspect(aspect, entry);
		}
	}
	return(0);
}

/* Timer */

int l_create_timer(lua_State * lua) {
	if(not lua_isnumber(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("create_timer(duration, script)");
		lua_pushnil(lua);
	} else {
		int duration = lua_tointeger(lua, 1);
		Script script { lua_tostring(lua, 2) };
		Uuid id = Luawrapper::server->addTimer(duration, script);
		lua_pushstring(lua, id.toString().c_str());
	}
	return(1);
}

int l_delete_timer(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("delete_timer(timer_id)");
	} else {
		Uuid id { lua_tostring(lua, 1) };
		Luawrapper::server->delTimer(id);
	}
	return(0);
}

int l_timer_getremaining(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("delete_timer(timer_id)");
		lua_pushnil(lua);
	} else {
		Uuid id { lua_tostring(lua, 1) };
		lua_pushinteger(lua, Luawrapper::server->getTimerRemaining(id));
	}
	return(1);
}

int l_timer_setremaining(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isnumber(lua, 2)) {
		lua_arg_error("timer_setremaining(timer_id, val)");
	} else {
		Uuid id { lua_tostring(lua, 1) };
		int remaining = lua_tointeger(lua, 2);
		Luawrapper::server->setTimerRemaining(id, remaining);
	}
	return(0);
}

int l_timer_triggernow(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("timer_triggernow(timer_id)");
	} else {
		Uuid id { lua_tostring(lua, 1) };
		Luawrapper::server->triggerTimer(id);
	}
	return(0);
}

/* Zone */

int l_new_zone(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isstring(lua, 2)
			or not lua_isnumber(lua, 3)
			or not lua_isnumber(lua, 4)
			or not lua_isstring(lua, 5)) {
		lua_arg_error("new_zone(zone_id, name, width, height, aspect)");
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		Name name { lua_tostring(lua, 2) };
		unsigned int width = lua_tointeger(lua, 3);
		unsigned int height = lua_tointeger(lua, 4);
		Aspect aspect { lua_tostring(lua, 5) };
		new Zone(Luawrapper::server, zone_id, name, width, height, aspect);
	}
	return(0);
}

int l_assert_zone(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("assert_zone(zone_id)");
		lua_pushnil(lua);
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone == nullptr) {
			lua_pushboolean(lua, false);
		} else {
			lua_pushboolean(lua, true);
		}
	}
	return(1);
}

int l_zone_getname(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("zone_getname(zone_id)");
		lua_pushnil(lua);
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			lua_pushstring(lua, zone->getName().toString().c_str());
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_zone_setname(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("zone_setname(zone_id, name)");
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			std::string name = lua_tostring(lua, 2);
			zone->setName(Name{name});
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
		}
	}
	return(0);
}

int l_zone_getwidth(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("zone_getwidth(zone_id)");
		lua_pushnil(lua);
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			lua_pushinteger(lua, zone->getWidth());
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_zone_getheight(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("zone_getheight(zone_id)");
		lua_pushnil(lua);
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			lua_pushinteger(lua, zone->getHeight());
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_zone_event(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("zone_event(zone_id, message)");
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			std::string message = lua_tostring(lua, 2);
			zone->event(message);
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
		}
	}
	return(0);
}

/* Place */

int l_place_getaspect(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isnumber(lua, 2)
			or not lua_isnumber(lua, 3)) {
		lua_arg_error("place_getaspect(zone_id, x, y)");
		lua_pushnil(lua);
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			unsigned int x = lua_tointeger(lua, 2);
			unsigned int y = lua_tointeger(lua, 3);
			class Place * place = zone->getPlace(x, y);
			if(place != nullptr) {
				lua_pushstring(lua, place->getAspect().toString().c_str());
			} else {
				warning("Invalid place "
					+ std::to_string(x) + "-" + std::to_string(y)
					+ " in zone '" + zone_id + "'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_place_setaspect(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isnumber(lua, 2)
			or not lua_isnumber(lua, 3)
			or not lua_isstring(lua, 4)) {
		lua_arg_error("place_setaspect(zone_id, x, y, aspect)");
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			unsigned int x = lua_tointeger(lua, 2);
			unsigned int y = lua_tointeger(lua, 3);
			class Place * place = zone->getPlace(x, y);
			if(place != nullptr) {
				// Set aspect.
				Aspect aspect { lua_tostring(lua, 4) };
				place->setAspect(aspect);

				// Set default passability.
				if(Aspect::getAspectDefaultPassable(aspect)) {
					place->setWalkable();
				} else {
					place->setNotWalkable();
				}

				// Update place aspect.
				zone->updatePlaceAspect(x, y);
			} else {
				warning("Invalid place "
					+ std::to_string(x) + "-" + std::to_string(y)
					+ " in zone '" + zone_id + "'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
		}
	}
	return(0);
}

int l_place_ispassable(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isnumber(lua, 2)
			or not lua_isnumber(lua, 3)) {
		lua_arg_error("place_ispassable(zone_id, x, y)");
		lua_pushnil(lua);
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			unsigned int x = lua_tointeger(lua, 2);
			unsigned int y = lua_tointeger(lua, 3);
			class Place * place = zone->getPlace(x, y);
			if(place != nullptr) {
				lua_pushboolean(lua, place->isWalkable());
			} else {
				warning("Invalid place "
					+ std::to_string(x) + "-" + std::to_string(y)
					+ " in zone '" + zone_id + "'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_place_setpassable(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isnumber(lua, 2)
			or not lua_isnumber(lua, 3)) {
		lua_arg_error("place_setpassable(zone_id, x, y)");
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			unsigned int x = lua_tointeger(lua, 2);
			unsigned int y = lua_tointeger(lua, 3);
			class Place * place = zone->getPlace(x, y);
			if(place != nullptr) {
				place->setWalkable();
			} else {
				warning("Invalid place "
					+ std::to_string(x) + "-" + std::to_string(y)
					+ " in zone '" + zone_id + "'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
		}
	}
	return(0);
}

int l_place_setnotpassable(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isnumber(lua, 2)
			or not lua_isnumber(lua, 3)) {
		lua_arg_error("place_setnotpassable(zone_id, x, y)");
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			unsigned int x = lua_tointeger(lua, 2);
			unsigned int y = lua_tointeger(lua, 3);
			class Place * place = zone->getPlace(x, y);
			if(place != nullptr) {
				place->setNotWalkable();
			} else {
				warning("Invalid place "
					+ std::to_string(x) + "-" + std::to_string(y)
					+ " in zone '" + zone_id + "'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
		}
	}
	return(0);
}

int l_place_getlandon(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isnumber(lua, 2)
			or not lua_isnumber(lua, 3)) {
		lua_arg_error("place_getlandon(zone_id, x, y)");
		lua_pushnil(lua);
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			unsigned int x = lua_tointeger(lua, 2);
			unsigned int y = lua_tointeger(lua, 3);
			class Place * place = zone->getPlace(x, y);
			if(place != nullptr) {
				lua_pushstring(lua, place->getWhenWalkedOn().toString().c_str());
			} else {
				warning("Invalid place "
					+ std::to_string(x) + "-" + std::to_string(y)
					+ " in zone '" + zone_id + "'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_place_setlandon(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isnumber(lua, 2)
			or not lua_isnumber(lua, 3)
			or not lua_isstring(lua, 4)) {
		lua_arg_error("place_setlandon(zone_id, x, y, script)");
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			unsigned int x = lua_tointeger(lua, 2);
			unsigned int y = lua_tointeger(lua, 3);
			class Place * place = zone->getPlace(x, y);
			if(place != nullptr) {
				Script script { lua_tostring(lua, 4) };
				place->setWhenWalkedOn(script);
			} else {
				warning("Invalid place "
					+ std::to_string(x) + "-" + std::to_string(y)
					+ " in zone '" + zone_id + "'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
		}
	}
	return(0);
}

int l_place_resetlandon(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isnumber(lua, 2)
			or not lua_isnumber(lua, 3)) {
		lua_arg_error("place_resetlandon(zone_id, x, y)");
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			unsigned int x = lua_tointeger(lua, 2);
			unsigned int y = lua_tointeger(lua, 3);
			class Place * place = zone->getPlace(x, y);
			if(place != nullptr) {
				place->resetWhenWalkedOn();
			} else {
				warning("Invalid place "
					+ std::to_string(x) + "-" + std::to_string(y)
					+ " in zone '" + zone_id + "'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
		}
	}
	return(0);
}

int l_place_gettag(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isnumber(lua, 2)
			or not lua_isnumber(lua, 3)
			or not lua_isstring(lua, 3)) {
		lua_arg_error("place_gettag(zone_id, x, y, tag_id)");
		lua_pushnil(lua);
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			int x = lua_tointeger(lua, 2);
			int y = lua_tointeger(lua, 3);
			Place * place = zone->getPlace(x, y);
			if(place != nullptr) {
				TagID tag_id { lua_tostring(lua, 4) };
				lua_pushstring(lua, place->getTag(tag_id).toString().c_str());
			} else {
				warning("Invalid place "
					+ std::to_string(x) + "-" + std::to_string(y)
					+ " in zone '" + zone_id + "'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_place_settag(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isnumber(lua, 2)
			or not lua_isnumber(lua, 3)
			or not lua_isstring(lua, 4)
			or not lua_isstring(lua, 5)) {
		lua_arg_error("place_settag(zone_id, x, y, tag_id, value)");
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			int x = lua_tointeger(lua, 2);
			int y = lua_tointeger(lua, 3);
			Place * place = zone->getPlace(x, y);
			if(place != nullptr) {
				TagID tag_id = TagID { lua_tostring(lua, 4) };
				TagValue value = TagValue { lua_tostring(lua, 5) };
				place->setTag(tag_id, value);
			} else {
				warning("Invalid place "
					+ std::to_string(x) + "-" + std::to_string(y)
					+ " in zone '" + zone_id + "'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
		}
	}
	return(0);
}

int l_place_deltag(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isnumber(lua, 2)
			or not lua_isnumber(lua, 3)
			or not lua_isstring(lua, 4)) {
		lua_arg_error("place_deltag(zone_id, x, y, tag_id)");
	} else {
		std::string zone_id = lua_tostring(lua, 1);
		class Zone * zone = Luawrapper::server->getZone(zone_id);
		if(zone != nullptr) {
			int x = lua_tointeger(lua, 2);
			int y = lua_tointeger(lua, 3);
			Place * place = zone->getPlace(x, y);
			if(place != nullptr) {
				TagID tag_id = TagID { lua_tostring(lua, 4) };
				place->delTag(tag_id);
			} else {
				warning("Invalid place "
					+ std::to_string(x) + "-" + std::to_string(y)
					+ " in zone '" + zone_id + "'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Zone '"+zone_id+"' doesn't exist.");
		}
	}
	return(0);
}

/* Player */

int l_delete_player(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("delete_player(player_id)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			delete(player);
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_assert_player(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("assert_player(player_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player == nullptr) {
			lua_pushboolean(lua, false);
		} else {
			lua_pushboolean(lua, true);
		}
	}
	return(1);
}

int l_player_spawn(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isstring(lua, 2)
			or not lua_isnumber(lua, 3)
			or not lua_isnumber(lua, 4)) {
		lua_arg_error("player_spawn(player_id, zone_id, x, y)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			std::string zone_id = lua_tostring(lua, 2);
			class Zone * zone = Luawrapper::server->getZone(zone_id);
			if(zone != nullptr) {
				int x = lua_tointeger(lua, 3);
				int y = lua_tointeger(lua, 4);
				player->spawn(zone, x, y);
			} else {
				warning("Zone '"+zone_id+"' doesn't exist.");
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_player_getname(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("player_getname(player_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			lua_pushstring(lua, player->getName().toString().c_str());
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_player_setname(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("player_setname(player_id, name)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			std::string name = lua_tostring(lua, 2);
			player->setName(Name{name});
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_player_getaspect(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("player_getaspect(player_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			lua_pushstring(lua, player->getAspect().toString().c_str());
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_player_setaspect(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("player_setaspect(player_id, aspect)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Aspect aspect { lua_tostring(lua, 2) };
			player->setAspect(aspect);

			// Update aspect.
			class Zone * zone = player->getZone();
			if(zone != nullptr) {
				zone->updatePlayer(player);
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_player_getzone(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("player_getzone(player_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			lua_pushstring(lua, player->getZone()->getId().c_str());
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_player_getx(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("player_getx(player_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			lua_pushinteger(lua, player->getX());
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_player_gety(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("player_gety(player_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			lua_pushinteger(lua, player->getY());
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_player_setxy(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isnumber(lua, 2)
			or not lua_isnumber(lua, 3)) {
		lua_arg_error("player_setxy(player_id, x, y)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			int x = lua_tointeger(lua, 2);
			int y = lua_tointeger(lua, 3);
			player->setXY(x, y);
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_player_move(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isnumber(lua, 2)
			or not lua_isnumber(lua, 3)) {
		lua_arg_error("player_move(player_id, x_shift, y_shift)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			int x = lua_tointeger(lua, 2);
			int y = lua_tointeger(lua, 3);
			player->move(x, y);
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_player_changezone(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isstring(lua, 2)
			or not lua_isnumber(lua, 3)
			or not lua_isnumber(lua, 4)) {
		lua_arg_error("player_changezone(player_id, zone_id, x, y)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			std::string zone_id = lua_tostring(lua, 2);
			class Zone * zone = Luawrapper::server->getZone(zone_id);
			if(zone != nullptr) {
				int x = lua_tointeger(lua, 3);
				int y = lua_tointeger(lua, 4);
				player->changeZone(zone, x, y);
			} else {
				warning("Zone '"+zone_id+"' doesn't exist.");
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_player_getwhendeath(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("player_getwhendeath(player_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			lua_pushstring(lua, player->getWhenDeath().toString().c_str());
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_player_setwhendeath(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("player_setwhendeath(player_id, script)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Script script = Script { lua_tostring(lua, 2) };
			player->setWhenDeath(script);
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_player_delgauge(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("player_delgauge(player_id, gauge_id)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			player->delGauge(name);
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_player_gettag(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("player_gettag(player_id, tag_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			TagID tag_id = TagID { lua_tostring(lua, 2) };
			lua_pushstring(lua, player->getTag(tag_id).toString().c_str());
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_player_settag(lua_State * lua) {
	if(not lua_isstring(lua, 1)
			or not lua_isstring(lua, 2)
			or not lua_isstring(lua, 3)) {
		lua_arg_error("player_settag(player_id, tag_id, value)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			TagID tag_id = TagID { lua_tostring(lua, 2) };
			TagValue value = TagValue { lua_tostring(lua, 3) };
			player->setTag(tag_id, value);
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_player_deltag(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("player_deltag(player_id, tag_id)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			TagID tag_id = TagID { lua_tostring(lua, 2) };
			player->delTag(tag_id);
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_player_isghost(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("player_isghost(player_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			lua_pushboolean(lua, player->isGhost());
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_player_setghost(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isboolean(lua, 2)) {
		lua_arg_error("player_setghost(player_id, bool)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			bool b = lua_toboolean(lua, 2);
			b ? player->setGhost() : player->setNotGhost();
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_player_message(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("player_message(player_id, message)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			std::string message = lua_tostring(lua, 2);
			player->message(message);
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_player_follow(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("player_follow(player_id, target_player_id)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Uuid target_id { lua_tostring(lua, 2) };
			class Player * target = Luawrapper::server->getPlayer(target_id);
			if(target != nullptr) {
				player->follow(target);
			} else {
				warning("Player '"+target_id.toString()+"' doesn't exist.");
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_player_hint(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2) or not lua_isstring(lua, 3)) {
		lua_arg_error("player_hint(player_id, aspect, hint)");
		return(0);
	}

	Uuid player_id { lua_tostring(lua, 1) };
	class Player * player = Luawrapper::server->getPlayer(player_id);
	if(player == nullptr) {
		warning("Player '"+player_id.toString()+"' doesn't exist.");
		return(0);
	}

	Aspect aspect { lua_tostring(lua, 2) };

	player->hint(aspect, lua_tostring(lua, 3));

	return(0);
}

/* Gauge */

int l_new_gauge(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)
			or not lua_isnumber(lua, 3)
			or not lua_isnumber(lua, 4)
			or not lua_isstring(lua, 5)
			or not lua_isstring(lua, 6)) {
		lua_arg_error("new_gauge(player_id, gauge_id, val, max, aspectFull, aspectEmpty, [, visible])");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			std::string gauge_id = lua_tostring(lua, 2);
			unsigned int val = lua_tointeger(lua, 3);
			unsigned int max = lua_tointeger(lua, 4);
			Aspect aspectFull { lua_tostring(lua, 5) };
			Aspect aspectEmpty { lua_tostring(lua, 6) };
			if(lua_isboolean(lua, 7)) {
				new Gauge(player, Name{gauge_id}, val, max, aspectFull, aspectEmpty,
						lua_toboolean(lua, 7));
			} else {
				new Gauge(player, Name{gauge_id}, val, max, aspectFull, aspectEmpty);
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_assert_gauge(lua_State * lua) {
	if(not lua_isnumber(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("assert_gauge(player_id, gauge_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge == nullptr) {
				lua_pushboolean(lua, false);
			} else {
				lua_pushboolean(lua, true);
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_gauge_getname(lua_State * lua) {
	if(not lua_isnumber(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("gauge_getname(player_id, gauge_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				lua_pushstring(lua, gauge->getName().toString().c_str());
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_gauge_setname(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)
			or not lua_isstring(lua, 3)) {
		lua_arg_error("gauge_setname(player_id, gauge_id, name)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				Name new_name = Name { lua_tostring(lua, 3) };
				gauge->setName(new_name);
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_gauge_getval(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)) {
		lua_arg_error("gauge_getval(player_id, gauge_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				lua_pushinteger(lua, gauge->getVal());
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_gauge_setval(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)
			or not lua_isnumber(lua, 3)) {
		lua_arg_error("gauge_setval(player_id, gauge_id, val)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				unsigned int val = lua_tointeger(lua, 3);
				gauge->setVal(val);
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_gauge_increase(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)
			or not lua_isnumber(lua, 3)) {
		lua_arg_error("gauge_increase(player_id, gauge_id, val)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				unsigned int val = lua_tointeger(lua, 3);
				gauge->increase(val);
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_gauge_decrease(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)
			or not lua_isnumber(lua, 3)) {
		lua_arg_error("gauge_decrease(player_id, gauge_id, val)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				unsigned int val = lua_tointeger(lua, 3);
				gauge->decrease(val);
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_gauge_getmax(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)) {
		lua_arg_error("gauge_getmax(player_id, gauge_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				lua_pushinteger(lua, gauge->getMax());
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_gauge_setmax(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)
			or not lua_isnumber(lua, 3)) {
		lua_arg_error("gauge_setmax(player_id, gauge_id, max)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				unsigned int val = lua_tointeger(lua, 3);
				gauge->setMax(val);
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_gauge_getwhenfull(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)) {
		lua_arg_error("gauge_getwhenfull(player_id, gauge_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				lua_pushstring(lua, gauge->getWhenFull().toString().c_str());
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_gauge_setwhenfull(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)
			or not lua_isstring(lua, 3)) {
		lua_arg_error("gauge_setwhenfull(player_id, gauge_id, script)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				Script script = Script { lua_tostring(lua, 3) };
				gauge->setWhenFull(script);
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_gauge_resetwhenfull(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)) {
		lua_arg_error("gauge_resetwhenfull(player_id, gauge_id)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				gauge->resetWhenFull();
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_gauge_getwhenempty(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)) {
		lua_arg_error("gauge_getwhenempty(player_id, gauge_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				lua_pushstring(lua, gauge->getWhenEmpty().toString().c_str());
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
				lua_pushnil(lua);
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
			lua_pushnil(lua);
		}
	}
	return(1);
}

int l_gauge_setwhenempty(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)
			or not lua_isstring(lua, 3)) {
		lua_arg_error("gauge_setwhenempty(player_id, gauge_id, script)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				Script script = Script { lua_tostring(lua, 3) };
				gauge->setWhenEmpty(script);
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_gauge_resetwhenempty(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)) {
		lua_arg_error("gauge_resetwhenempty(player_id, gauge_id)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				gauge->resetWhenEmpty();
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}

int l_gauge_isvisible(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)) {
		lua_arg_error("gauge_isvisible(player_id, gauge_id)");
		lua_pushnil(lua);
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				lua_pushboolean(lua, gauge->isVisible());
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(1);
}

int l_gauge_setvisible(lua_State * lua) {
	if(not lua_isnumber(lua, 1)
			or not lua_isstring(lua, 2)
			or not lua_isboolean(lua, 3)) {
		lua_arg_error("gauge_setvisible(player_id, gauge_id, bool)");
	} else {
		Uuid player_id { lua_tostring(lua, 1) };
		class Player * player = Luawrapper::server->getPlayer(player_id);
		if(player != nullptr) {
			Name name = Name { lua_tostring(lua, 2) };
			class Gauge * gauge = player->getGauge(name);
			if(gauge != nullptr) {
				bool b = lua_toboolean(lua, 3);
				b ? gauge->setVisible() : gauge->setNotVisible();
			} else {
				warning("Player '"+player_id.toString()
						+"' doesn't have gauge '"+name.toString()+"'.");
			}
		} else {
			warning("Player '"+player_id.toString()+"' doesn't exist.");
		}
	}
	return(0);
}


/* Artifacts */

int l_create_artifact(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("create_artifact(name)");
		lua_pushnil(lua);
		return(1);
	}

	Uuid id = Luawrapper::server->newArtifact(Name{lua_tostring(lua, 1)});
	lua_pushstring(lua, id.toString().c_str());
	return(1);
}

int l_delete_artifact(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("delete_artifact(artifact_id)");
		return(0);
	}

	Uuid id{ lua_tostring(lua, 1) };
	Luawrapper::server->delArtifact(id);
	return(0);
}

int l_artifact_getname(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("artifact_getname(artifact_id)");
		lua_pushnil(lua);
		return(1);
	}

	Uuid id{ lua_tostring(lua, 1) };
	Artifact* artifact = Luawrapper::server->getArtifact(id);

	if(artifact == nullptr) {
		warning("Artifact "+id.toString()+" doesn't exist.");
		lua_pushnil(lua);
		return(1);
	}

	lua_pushstring(lua, artifact->getName().toString().c_str());
	return(1);
}

int l_artifact_setname(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("artifact_setname(artifact_id, name)");
		return(0);
	}

	Uuid id{ lua_tostring(lua, 1) };
	Artifact* artifact = Luawrapper::server->getArtifact(id);

	if(artifact == nullptr) {
		warning("Artifact "+id.toString()+" doesn't exist.");
		return(0);
	}

	artifact->setName(Name{ lua_tostring(lua, 2) });
	return(0);
}

int l_artifact_gettag(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("artifact_gettag(artifact_id, tag)");
		lua_pushnil(lua);
		return(1);
	}

	Uuid id{ lua_tostring(lua, 1) };
	Artifact* artifact = Luawrapper::server->getArtifact(id);

	if(artifact == nullptr) {
		warning("Artifact "+id.toString()+" doesn't exist.");
		lua_pushnil(lua);
		return(1);
	}

	lua_pushstring(lua, artifact->getTag(TagID{ lua_tostring(lua, 2) }).toString().c_str());
	return(1);
}

int l_artifact_settag(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2) or not lua_isstring(lua, 3)) {
		lua_arg_error("artifact_settag(artifact_id, tag, value)");
		return(0);
	}

	Uuid id{ lua_tostring(lua, 1) };
	Artifact* artifact = Luawrapper::server->getArtifact(id);

	if(artifact == nullptr) {
		warning("Artifact "+id.toString()+" doesn't exist.");
		return(0);
	}

	artifact->setTag(TagID{ lua_tostring(lua, 2) }, TagValue{ lua_tostring(lua, 3) });
	return(0);
}

int l_artifact_deltag(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("artifact_deltag(artifact_id, tag)");
		return(0);
	}

	Uuid id{ lua_tostring(lua, 1) };
	Artifact* artifact = Luawrapper::server->getArtifact(id);

	if(artifact == nullptr) {
		warning("Artifact "+id.toString()+" doesn't exist.");
		return(0);
	}

	artifact->delTag(TagID{ lua_tostring(lua, 2) });
	return(0);
}

/* Inventory */

int l_create_inventory(lua_State * lua) {
	if(not lua_isinteger(lua, 1)) {
		lua_arg_error("create_inventory(size)");
		lua_pushnil(lua);
		return(1);
	}

	unsigned int size = lua_tointeger(lua, 1);
	Uuid id = Luawrapper::server->newInventory(size);
	lua_pushstring(lua, id.toString().c_str());
	return(1);
}

int l_delete_inventory(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("delete_inventory(inventory_id)");
		return(0);
	}

	Uuid id { lua_tostring(lua, 1) };
	Luawrapper::server->delInventory(id);
	return(0);
}

int l_inventory_get(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isstring(lua, 2)) {
		lua_arg_error("inventory_get(inventory_id, item_name)");
		lua_pushnil(lua);
		return(1);
	}

	Uuid id { lua_tostring(lua, 1) };
	Inventory* inventory = Luawrapper::server->getInventory(id);
	if(inventory == nullptr) {
		warning("Inventory "+id.toString()+" doesn't exist.");
		lua_pushnil(lua);
		return(1);
	}

	std::string name { lua_tostring(lua, 2) };
	unsigned int returned = inventory->get(name);
	lua_pushinteger(lua, returned);
	return(1);
}

int l_inventory_get_all(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("inventory_get_all(inventory_id)");
		lua_pushnil(lua);
		return(1);
	}

	Uuid id { lua_tostring(lua, 1) };
	Inventory* inventory = Luawrapper::server->getInventory(id);
	if(inventory == nullptr) {
		warning("Inventory "+id.toString()+" doesn't exist.");
		lua_pushnil(lua);
		return(1);
	}

	std::vector<std::string> items = inventory->get_all();
	lua_newtable(lua);

	for(std::string item : items) {
		lua_pushstring(lua, item.c_str());
		lua_pushinteger(lua, inventory->get(item));
		lua_settable(lua, -3);
	}
	return(1);
}

int l_inventory_size(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("inventory_size(inventory_id)");
		lua_pushnil(lua);
		return(1);
	}

	Uuid id { lua_tostring(lua, 1) };
	Inventory* inventory = Luawrapper::server->getInventory(id);
	if(inventory == nullptr) {
		warning("Inventory "+id.toString()+" doesn't exist.");
		lua_pushnil(lua);
		return(1);
	}

	unsigned int size = inventory->size();
	lua_pushinteger(lua, size);
	return(1);
}

int l_inventory_resize(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isinteger(lua, 2)) {
		lua_arg_error("inventory_resize(inventory_id, size)");
		return(0);
	}

	Uuid id { lua_tostring(lua, 1) };
	Inventory* inventory = Luawrapper::server->getInventory(id);
	if(inventory == nullptr) {
		warning("Inventory "+id.toString()+" doesn't exist.");
		return(0);
	}

	unsigned int size = lua_tointeger(lua, 2);
	inventory->resize(size);
	return(0);
}

int l_inventory_available(lua_State * lua) {
	if(not lua_isstring(lua, 1)) {
		lua_arg_error("inventory_available(inventory_id)");
		lua_pushnil(lua);
		return(1);
	}

	Uuid id { lua_tostring(lua, 1) };
	Inventory* inventory = Luawrapper::server->getInventory(id);
	if(inventory == nullptr) {
		warning("Inventory "+id.toString()+" doesn't exist.");
		lua_pushnil(lua);
		return(1);
	}

	lua_pushinteger(lua, inventory->available());
	return(1);
}

int l_inventory_add(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isinteger(lua, 2) or not lua_isstring(lua, 3)) {
		lua_arg_error("inventory_add(inventory_id, int quantity, item_name)");
		lua_pushnil(lua);
		return(1);
	}

	Uuid id { lua_tostring(lua, 1) };
	Inventory* inventory = Luawrapper::server->getInventory(id);
	if(inventory == nullptr) {
		warning("Inventory "+id.toString()+" doesn't exist.");
		lua_pushnil(lua);
		return(1);
	}

	unsigned int quantity = lua_tointeger(lua, 2);
	std::string name { lua_tostring(lua, 3) };
	unsigned int returned = inventory->add(quantity, name);
	lua_pushinteger(lua, returned);
	return(1);
}

int l_inventory_add_all(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isinteger(lua, 2) or not lua_isstring(lua, 3)) {
		lua_arg_error("inventory_add_all(inventory_id, int quantity, item_name)");
		lua_pushnil(lua);
		return(1);
	}

	Uuid id { lua_tostring(lua, 1) };
	Inventory* inventory = Luawrapper::server->getInventory(id);
	if(inventory == nullptr) {
		warning("Inventory "+id.toString()+" doesn't exist.");
		lua_pushnil(lua);
		return(1);
	}

	unsigned int quantity = lua_tointeger(lua, 2);
	std::string name { lua_tostring(lua, 3) };
	unsigned int returned = inventory->add_all(quantity, name);
	lua_pushinteger(lua, returned);
	return(1);
}

int l_inventory_del(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isinteger(lua, 2) or not lua_isstring(lua, 3)) {
		lua_arg_error("inventory_del(inventory_id, int quantity, item_name)");
		lua_pushnil(lua);
		return(1);
	}

	Uuid id { lua_tostring(lua, 1) };
	Inventory* inventory = Luawrapper::server->getInventory(id);
	if(inventory == nullptr) {
		warning("Inventory "+id.toString()+" doesn't exist.");
		lua_pushnil(lua);
		return(1);
	}

	unsigned int quantity = lua_tointeger(lua, 2);
	std::string name { lua_tostring(lua, 3) };
	unsigned int returned = inventory->del(quantity, name);
	lua_pushinteger(lua, returned);
	return(1);
}

int l_inventory_del_all(lua_State * lua) {
	if(not lua_isstring(lua, 1) or not lua_isinteger(lua, 2) or not lua_isstring(lua, 3)) {
		lua_arg_error("inventory_del_all(inventory_id, int quantity, item_name)");
		lua_pushnil(lua);
		return(1);
	}

	Uuid id { lua_tostring(lua, 1) };
	Inventory* inventory = Luawrapper::server->getInventory(id);
	if(inventory == nullptr) {
		warning("Inventory "+id.toString()+" doesn't exist.");
		lua_pushnil(lua);
		return(1);
	}

	unsigned int quantity = lua_tointeger(lua, 2);
	std::string name { lua_tostring(lua, 3) };
	unsigned int returned = inventory->del_all(quantity, name);
	lua_pushinteger(lua, returned);
	return(1);
}

int l_inventory_move(lua_State * lua) {
	if(not lua_isstring(lua, 1)
		or not lua_isinteger(lua, 2)
		or not lua_isstring(lua, 3)
		or not lua_isstring(lua, 4)
	) {
		lua_arg_error("inventory_move(inventory_id, int quantity, item_name, inventory_id)");
		lua_pushnil(lua);
		return(1);
	}

	Uuid id { lua_tostring(lua, 1) };
	Inventory* inventory = Luawrapper::server->getInventory(id);
	if(inventory == nullptr) {
		warning("Inventory "+id.toString()+" doesn't exist.");
		lua_pushnil(lua);
		return(1);
	}

	Uuid dst_id { lua_tostring(lua, 1) };
	Inventory* dst_inventory = Luawrapper::server->getInventory(id);
	if(dst_inventory == nullptr) {
		warning("Inventory "+dst_id.toString()+" doesn't exist.");
		lua_pushnil(lua);
		return(1);
	}

	unsigned int quantity = lua_tointeger(lua, 2);
	std::string name { lua_tostring(lua, 3) };
	unsigned int returned = inventory->move(quantity, name, *dst_inventory);
	lua_pushinteger(lua, returned);
	return(1);
}

int l_inventory_move_all(lua_State * lua) {
	if(not lua_isstring(lua, 1)
		or not lua_isinteger(lua, 2)
		or not lua_isstring(lua, 3)
		or not lua_isstring(lua, 4)
	) {
		lua_arg_error("inventory_move_all(inventory_id, int quantity, item_name, inventory_id)");
		lua_pushnil(lua);
		return(1);
	}

	Uuid id { lua_tostring(lua, 1) };
	Inventory* inventory = Luawrapper::server->getInventory(id);
	if(inventory == nullptr) {
		warning("Inventory "+id.toString()+" doesn't exist.");
		lua_pushnil(lua);
		return(1);
	}

	Uuid dst_id { lua_tostring(lua, 1) };
	Inventory* dst_inventory = Luawrapper::server->getInventory(id);
	if(dst_inventory == nullptr) {
		warning("Inventory "+dst_id.toString()+" doesn't exist.");
		lua_pushnil(lua);
		return(1);
	}

	unsigned int quantity = lua_tointeger(lua, 2);
	std::string name { lua_tostring(lua, 3) };
	unsigned int returned = inventory->move_all(quantity, name, *dst_inventory);
	lua_pushinteger(lua, returned);
	return(1);
}

/* Wraper class */

Luawrapper::Luawrapper(class Server * server) :
	lua_state(luaL_newstate())
{
	Luawrapper::server = server;
	luaL_openlibs(this->lua_state);

	lua_register(this->lua_state, "c_rand", l_c_rand);

	lua_register(this->lua_state, "setverbose", l_setverbose);
	lua_register(this->lua_state, "setnoverbose", l_setnoverbose);
	lua_register(this->lua_state, "isverbose", l_isverbose);
	lua_register(this->lua_state, "info", l_info);
	lua_register(this->lua_state, "warning", l_warning);
	lua_register(this->lua_state, "fatal", l_fatal);

	lua_register(this->lua_state, "halt", l_halt);
	lua_register(this->lua_state, "open", l_open);
	lua_register(this->lua_state, "close", l_close);
	lua_register(this->lua_state, "is_open", l_is_open);
	lua_register(this->lua_state, "get_port", l_get_port);
	lua_register(this->lua_state, "delete_zone", l_delete_zone);
	lua_register(this->lua_state, "add_action", l_add_action);
	lua_register(this->lua_state, "get_action", l_get_action);
	lua_register(this->lua_state, "delete_action", l_delete_action);
	lua_register(this->lua_state, "register_aspect", l_register_aspect);

	lua_register(this->lua_state, "create_timer", l_create_timer);
	lua_register(this->lua_state, "delete_timer", l_delete_timer);
	lua_register(this->lua_state, "timer_getremaining", l_timer_getremaining);
	lua_register(this->lua_state, "timer_setremaining", l_timer_setremaining);
	lua_register(this->lua_state, "timer_triggernow", l_timer_triggernow);

	lua_register(this->lua_state, "new_zone", l_new_zone);
	lua_register(this->lua_state, "assert_zone", l_assert_zone);
	lua_register(this->lua_state, "zone_getname", l_zone_getname);
	lua_register(this->lua_state, "zone_setname", l_zone_setname);
	lua_register(this->lua_state, "zone_getwidth", l_zone_getwidth);
	lua_register(this->lua_state, "zone_getheight", l_zone_getheight);
	lua_register(this->lua_state, "zone_event", l_zone_event);

	lua_register(this->lua_state, "place_getaspect", l_place_getaspect);
	lua_register(this->lua_state, "place_setaspect", l_place_setaspect); // Automatically set aspect's default passability.
	lua_register(this->lua_state, "place_ispassable", l_place_ispassable);
	lua_register(this->lua_state, "place_setpassable", l_place_setpassable);
	lua_register(this->lua_state, "place_setnotpassable", l_place_setnotpassable);
	lua_register(this->lua_state, "place_getlandon", l_place_getlandon);
	lua_register(this->lua_state, "place_setlandon", l_place_setlandon);
	lua_register(this->lua_state, "place_resetlandon", l_place_resetlandon);
	lua_register(this->lua_state, "place_gettag", l_place_gettag);
	lua_register(this->lua_state, "place_settag", l_place_settag);
	lua_register(this->lua_state, "place_deltag", l_place_deltag);

	lua_register(this->lua_state, "delete_player", l_delete_player);
	lua_register(this->lua_state, "assert_player", l_assert_player);
	lua_register(this->lua_state, "player_spawn", l_player_spawn);
	lua_register(this->lua_state, "player_getname", l_player_getname);
	lua_register(this->lua_state, "player_setname", l_player_setname);
	lua_register(this->lua_state, "player_getaspect", l_player_getaspect);
	lua_register(this->lua_state, "player_setaspect", l_player_setaspect);
	lua_register(this->lua_state, "player_getzone", l_player_getzone);
	lua_register(this->lua_state, "player_getx", l_player_getx);
	lua_register(this->lua_state, "player_gety", l_player_gety);
	lua_register(this->lua_state, "player_setxy", l_player_setxy);
	lua_register(this->lua_state, "player_move", l_player_move);
	lua_register(this->lua_state, "player_changezone", l_player_changezone);
	lua_register(this->lua_state, "player_getwhendeath", l_player_getwhendeath);
	lua_register(this->lua_state, "player_setwhendeath", l_player_setwhendeath);
	lua_register(this->lua_state, "player_delgauge", l_player_delgauge);
	lua_register(this->lua_state, "player_gettag", l_player_gettag);
	lua_register(this->lua_state, "player_settag", l_player_settag);
	lua_register(this->lua_state, "player_deltag", l_player_deltag);

	lua_register(this->lua_state, "player_isghost", l_player_isghost);
	lua_register(this->lua_state, "player_setghost", l_player_setghost);
	lua_register(this->lua_state, "player_message", l_player_message);
	lua_register(this->lua_state, "player_follow", l_player_follow);
	lua_register(this->lua_state, "player_hint", l_player_hint);

	lua_register(this->lua_state, "new_gauge", l_new_gauge);
	lua_register(this->lua_state, "assert_gauge", l_assert_gauge);
	lua_register(this->lua_state, "gauge_getname", l_gauge_getname);
	lua_register(this->lua_state, "gauge_setname", l_gauge_setname);
	lua_register(this->lua_state, "gauge_getval", l_gauge_getval);
	lua_register(this->lua_state, "gauge_setval", l_gauge_setval);
	lua_register(this->lua_state, "gauge_increase", l_gauge_increase);
	lua_register(this->lua_state, "gauge_decrease", l_gauge_decrease);
	lua_register(this->lua_state, "gauge_getmax", l_gauge_getmax);
	lua_register(this->lua_state, "gauge_setmax", l_gauge_setmax);
	lua_register(this->lua_state, "gauge_getwhenfull", l_gauge_getwhenfull);
	lua_register(this->lua_state, "gauge_setwhenfull", l_gauge_setwhenfull);
	lua_register(this->lua_state, "gauge_resetwhenfull", l_gauge_resetwhenfull);
	lua_register(this->lua_state, "gauge_getwhenempty", l_gauge_getwhenempty);
	lua_register(this->lua_state, "gauge_setwhenempty", l_gauge_setwhenempty);
	lua_register(this->lua_state, "gauge_resetwhenempty", l_gauge_resetwhenempty);
	lua_register(this->lua_state, "gauge_isvisible", l_gauge_isvisible);
	lua_register(this->lua_state, "gauge_setvisible", l_gauge_setvisible);

	lua_register(this->lua_state, "create_artifact", l_create_artifact);
	lua_register(this->lua_state, "delete_artifact", l_delete_artifact);
	lua_register(this->lua_state, "artifact_getname", l_artifact_getname);
	lua_register(this->lua_state, "artifact_setname", l_artifact_setname);
	lua_register(this->lua_state, "artifact_gettag", l_artifact_gettag);
	lua_register(this->lua_state, "artifact_settag", l_artifact_settag);
	lua_register(this->lua_state, "artifact_deltag", l_artifact_deltag);

	lua_register(this->lua_state, "create_inventory", l_create_inventory);
	lua_register(this->lua_state, "delete_inventory", l_delete_inventory);
	lua_register(this->lua_state, "inventory_get", l_inventory_get);
	lua_register(this->lua_state, "inventory_get_all", l_inventory_get_all);
	lua_register(this->lua_state, "inventory_size", l_inventory_size);
	lua_register(this->lua_state, "inventory_resize", l_inventory_resize);
	lua_register(this->lua_state, "inventory_available", l_inventory_available);
	lua_register(this->lua_state, "inventory_add", l_inventory_add);
	lua_register(this->lua_state, "inventory_add_all", l_inventory_add_all);
	lua_register(this->lua_state, "inventory_del", l_inventory_del);
	lua_register(this->lua_state, "inventory_del_all", l_inventory_del_all);
	lua_register(this->lua_state, "inventory_move", l_inventory_move);
	lua_register(this->lua_state, "inventory_move_all", l_inventory_move_all);

	this->executeFile(LUA_INIT_SCRIPT);
}

Luawrapper::~Luawrapper() {
	lua_close(this->lua_state);
}

void Luawrapper::executeFile(std::string filename, class Player * player, std::string arg) {
	if(player) {
		lua_pushstring(this->lua_state, player->getId().toString().c_str());
	} else {
		lua_pushnil(this->lua_state);
	}
	lua_setglobal(this->lua_state, "Player");

	if(arg == "") {
		lua_pushnil(this->lua_state);
	} else {
		lua_pushstring(this->lua_state, arg.c_str());
	}
	lua_setglobal(this->lua_state, "Arg");

	luaL_dofile(this->lua_state, filename.c_str());
}

void Luawrapper::executeCode(std::string code, class Player * player, std::string arg) {
	if(player) {
		lua_pushstring(this->lua_state, player->getId().toString().c_str());
	} else {
		lua_pushnil(this->lua_state);
	}
	lua_setglobal(this->lua_state, "Player");

	if(arg == "") {
		lua_pushnil(this->lua_state);
	} else {
		lua_pushstring(this->lua_state, arg.c_str());
	}
	lua_setglobal(this->lua_state, "Arg");

	luaL_dostring(this->lua_state, code.c_str());
}

void Luawrapper::spawnScript(class Player * player) {
	this->executeFile(LUA_SPAWN_SCRIPT, player);
}

