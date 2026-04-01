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

class Corpse;
class Lua_Client;
struct Lua_Corpse_Loot_List;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_corpse();
luabind::scope lua_register_corpse_loot_list();

class Lua_Corpse : public Lua_Mob
{
	typedef Corpse NativeType;
public:
	Lua_Corpse() { SetLuaPtrData(nullptr); }
	Lua_Corpse(Corpse *d) { SetLuaPtrData(reinterpret_cast<Entity*>(d)); }
	virtual ~Lua_Corpse() { }

	operator Corpse*() {
		return reinterpret_cast<Corpse*>(GetLuaPtrData());
	}

	uint32 GetCharID();
	uint32 GetDecayTime();
	void Lock();
	void UnLock();
	bool IsLocked();
	void ResetLooter();
	uint32 GetDBID();
	bool IsRezzed();
	const char *GetOwnerName();
	bool Save();
	void Delete();
	void Bury();
	void Depop();
	uint32 CountItems();
	void AddItem(uint32 itemnum, uint16 charges);
	void AddItem(uint32 itemnum, uint16 charges, int16 slot);
	void AddItem(uint32 itemnum, uint16 charges, int16 slot, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5);
	uint32 GetWornItem(int16 equipSlot);
	void RemoveItem(uint16 lootslot);
	void RemoveItemByID(uint32 item_id);
	void RemoveItemByID(uint32 item_id, int quantity);
	void SetCash(uint32 copper, uint32 silver, uint32 gold, uint32 platinum);
	void RemoveLootCash();
	bool IsEmpty();
	void ResetDecayTimer();
	void SetDecayTimer(uint32 decaytime);
	bool CanMobLoot(int charid);
	void AllowMobLoot(Lua_Mob them, uint8 slot);
	bool Summon(Lua_Client client, bool spell, bool checkdistance);
	uint32 GetCopper();
	uint32 GetSilver();
	uint32 GetGold();
	uint32 GetPlatinum();
	void AddLooter(Lua_Mob who);
	bool HasItem(uint32 item_id);
	uint32 CountItem(uint32 item_id);
	uint32 GetItemIDBySlot(uint16 loot_slot);
	uint16 GetFirstLootSlotByItemID(uint32 item_id);
	Lua_Corpse_Loot_List GetLootList(lua_State* L);
};

#endif // LUA_EQEMU
