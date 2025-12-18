#pragma once

#ifdef LUA_EQEMU

#include "zone/lua_ptr.h"

class Encounter;

namespace luabind {
	struct scope;
	namespace adl {
		class object;
	}
}

luabind::scope lua_register_encounter();

class Lua_Encounter : public Lua_Ptr<Encounter>
{
	typedef Encounter NativeType;
public:
	Lua_Encounter() { SetLuaPtrData(nullptr); }
	Lua_Encounter(Encounter *d) { SetLuaPtrData(reinterpret_cast<Encounter*>(d)); }
	virtual ~Lua_Encounter() { }

	operator Encounter*() {
		return reinterpret_cast<Encounter*>(GetLuaPtrData());
	}

};

#endif // LUA_EQEMU
