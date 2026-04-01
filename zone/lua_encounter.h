/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
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
