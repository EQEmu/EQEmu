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

class Raid;
class Lua_Client;
class Lua_Mob;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_raid();

class Lua_Raid : public Lua_Ptr<Raid>
{
	typedef Raid NativeType;
public:
	Lua_Raid() : Lua_Ptr(nullptr) { }
	Lua_Raid(Raid *d) : Lua_Ptr(d) { }
	virtual ~Lua_Raid() { }

	operator Raid*() {
		return reinterpret_cast<Raid*>(GetLuaPtrData());
	}

	bool IsRaidMember(const char *name);
	bool IsRaidMember(Lua_Client c);
	void CastGroupSpell(Lua_Mob caster, int spell_id, uint32 group_id);
	int GroupCount(uint32 group_id);
	int RaidCount();
	int GetGroup(const char *c);
	int GetGroup(Lua_Client c);
	void SplitExp(uint64 exp, Lua_Mob other);
	uint32 GetTotalRaidDamage(Lua_Mob other);
	void SplitMoney(uint32 gid, uint32 copper, uint32 silver, uint32 gold, uint32 platinum);
	void SplitMoney(uint32 gid, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Lua_Client splitter);
	void BalanceHP(int penalty, uint32 group_id);
	bool IsLeader(const char *c);
	bool IsLeader(Lua_Client c);
	Lua_Client GetLeader();
	std::string GetLeaderName();
	bool IsGroupLeader(const char *name);
	bool IsGroupLeader(Lua_Client c);
	int GetHighestLevel();
	int GetLowestLevel();
	Lua_Client GetClientByIndex(int member_index);
	void TeleportGroup(Lua_Mob sender, uint32 zone_id, uint32 instance_id, float x, float y, float z, float h, uint32 group_id);
	void TeleportRaid(Lua_Mob sender, uint32 zone_id, uint32 instance_id, float x, float y, float z, float h);
	int GetID();
	Lua_Client GetMember(int member_index);
	int GetGroupNumber(int member_index);
	bool DoesAnyMemberHaveExpeditionLockout(std::string expedition_name, std::string event_name);
	bool DoesAnyMemberHaveExpeditionLockout(std::string expedition_name, std::string event_name, int max_check_count);
};

#endif // LUA_EQEMU
