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

#include "zone/common.h"
#include "zone/lua_ptr.h"

struct Buffs_Struct;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_buff();

class Lua_Buff : public Lua_Ptr<const Buffs_Struct>
{
	typedef const Buffs_Struct NativeType;
public:
	Lua_Buff() : Lua_Ptr(nullptr) { }
	Lua_Buff(const Buffs_Struct *d) : Lua_Ptr(d) { }
	virtual ~Lua_Buff() { }

	operator const Buffs_Struct*() {
		return reinterpret_cast<const Buffs_Struct*>(GetLuaPtrData());
	}

	uint16 GetCasterID();
	uint8 GetCasterLevel();
	std::string GetCasterName();
	int GetCastOnX();
	int GetCastOnY();
	int GetCastOnZ();
	uint32 GetCounters();
	uint32 GetDOTRune();
	int GetExtraDIChance();
	uint32 GetInstrumentModifier();
	uint32 GetMagicRune();
	uint32 GetMeleeRune();
	uint32 GetNumberOfHits();
	int16 GetRootBreakChance();
	uint16 GetSpellID();
	int GetTicsRemaining();
	int GetVirusSpreadTime();
	bool IsCasterClient();
	bool IsPersistentBuff();
	bool SendsClientUpdate();
};

#endif // LUA_EQEMU
