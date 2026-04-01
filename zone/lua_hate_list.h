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

class Lua_Mob;
struct struct_HateList;

luabind::scope lua_register_hate_entry();
luabind::scope lua_register_hate_list();

class Lua_HateEntry : public Lua_Ptr<struct_HateList>
{
	typedef struct_HateList NativeType;
public:
	Lua_HateEntry() : Lua_Ptr(nullptr) { }
	Lua_HateEntry(struct_HateList *d) : Lua_Ptr(d) { }
	virtual ~Lua_HateEntry() { }

	Lua_Mob GetEnt();
	void SetEnt(Lua_Mob e);
	int64 GetDamage();
	void SetDamage(int64 value);
	int64 GetHate();
	void SetHate(int64 value);
	bool GetFrenzy();
	void SetFrenzy(bool value);
};

struct Lua_HateList
{
	std::vector<Lua_HateEntry> entries;
};

#endif // LUA_EQEMU
