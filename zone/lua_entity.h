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

class Entity;
class Lua_Client;
class Lua_Bot;
class Lua_Merc;
class Lua_NPC;
class Lua_Mob;
struct Lua_HateList;
class Lua_Item;
class Lua_ItemInst;
class Lua_Corpse;
class Lua_Object;
class Lua_Door;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_entity();

class Lua_Entity : public Lua_Ptr<Entity>
{
	typedef Entity NativeType;
public:
	Lua_Entity() : Lua_Ptr(nullptr) { }
	Lua_Entity(Entity *d) : Lua_Ptr(d) { }
	virtual ~Lua_Entity() { }

	operator Entity*() {
		return reinterpret_cast<Entity*>(GetLuaPtrData());
	}

	bool IsClient();
	bool IsNPC();
	bool IsMob();
	bool IsMerc();
	bool IsCorpse();
	bool IsPlayerCorpse();
	bool IsNPCCorpse();
	bool IsObject();
	bool IsDoor();
	bool IsTrap();
	bool IsBeacon();
	bool IsEncounter();
	bool IsBot();
	bool IsAura();
	bool IsOfClientBot();
	bool IsOfClientBotMerc();
	int GetID();

	Lua_Client CastToClient();
	Lua_NPC CastToNPC();
	Lua_Mob CastToMob();
	Lua_Corpse CastToCorpse();
	Lua_Object CastToObject();
	Lua_Door CastToDoor();
	Lua_Bot CastToBot();
	Lua_Merc CastToMerc();
};

#endif // LUA_EQEMU
