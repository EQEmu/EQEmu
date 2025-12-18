#ifdef LUA_EQEMU

#include "lua_encounter.h"

#include "zone/encounter.h"

#include "luabind/luabind.hpp"

luabind::scope lua_register_encounter() {
	return luabind::class_<Lua_Encounter>("Encounter");
}

#endif // LUA_EQEMU