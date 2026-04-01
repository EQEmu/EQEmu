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

#include "zone/lua_mob.h"

class Merc;
class Lua_Group;
class Lua_Merc;
class Lua_Mob;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_merc();

class Lua_Merc : public Lua_Mob
{
	typedef Merc NativeType;
public:
	Lua_Merc() { SetLuaPtrData(nullptr); }
	Lua_Merc(Merc *d) { SetLuaPtrData(reinterpret_cast<Entity*>(d)); }
	virtual ~Lua_Merc() { }

	operator Merc*() {
		return reinterpret_cast<Merc*>(GetLuaPtrData());
	}

	uint32 GetCostFormula();
	Lua_Group GetGroup();
	int GetHatedCount();
	float GetMaxMeleeRangeToTarget(Lua_Mob target);
	uint32 GetMercenaryCharacterID();
	uint32 GetMercenaryID();
	uint32 GetMercenaryNameType();
	Lua_Client GetMercenaryOwner();
	uint32 GetMercenarySubtype();
	uint32 GetMercenaryTemplateID();
	uint32 GetMercenaryType();
	Lua_Mob GetOwner();
	Lua_Mob GetOwnerOrSelf();
	uint8 GetProficiencyID();
	uint8 GetStance();
	uint8 GetTierID();
	bool HasOrMayGetAggro();
	bool IsMercenaryCaster();
	bool IsMercenaryCasterCombatRange(Lua_Mob target);
	bool IsSitting();
	bool IsStanding();
	void ScaleStats(int scale_percentage);
	void ScaleStats(int scale_percentage, bool set_to_max);
	void SendPayload(int payload_id, std::string payload_value);
	void SetTarget(Lua_Mob target);
	void Signal(int signal_id);
	void Sit();
	void Stand();
	bool Suspend();
	bool UseDiscipline(uint16 spell_id, uint16 target_id);
};

#endif // LUA_EQEMU
