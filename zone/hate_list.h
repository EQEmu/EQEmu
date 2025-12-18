/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY except by those people which sell it, which
are required to give you total support for your newly bought product;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#pragma once

#include "common/emu_constants.h"
#include "common/types.h"

#include <list>

class Client;
class Group;
class Mob;
class Raid;
struct ExtraAttackOptions;

struct struct_HateList {
	Mob    *entity_on_hatelist;
	int64  hatelist_damage;
	int64  stored_hate_amount;
	bool   is_entity_frenzy;
	int8   oor_count; // count on how long we've been out of range
	uint32 last_modified; // we need to remove this if it gets higher than 10 mins
};

enum class HateListCountType {
	Bot    = 0,
	Client = 1,
	NPC    = 2,
	All    = 3,
};

class HateList {
public:
	HateList();
	~HateList();

	Mob *GetClosestEntOnHateList(Mob *hater, bool skip_mezzed = false, EntityFilterType filter_type = EntityFilterType::All);
	Mob *GetDamageTopOnHateList(Mob *hater); // didn't add 'skip_mezzed' due to calls being in ::Death()
	Mob *GetMobWithMostHateOnList(Mob *center, Mob *skip = nullptr, bool skip_mezzed = false, EntityFilterType filter_type = EntityFilterType::All);
	Mob *GetRandomMobOnHateList(EntityFilterType filter_type = EntityFilterType::All);
	Mob *GetMobWithMostHateOnList(bool skip_mezzed = false);
	Mob *GetEscapingMobOnHateList(Mob *center, float range = 0.0f, bool first = false);

	bool IsEntOnHateList(Mob* m);
	bool IsHateListEmpty();
	bool RemoveEntFromHateList(Mob *ent);
	uint32 GetHateListCount(HateListCountType count_type = HateListCountType::All);

	int AreaRampage(Mob *caster, Mob *target, int count, ExtraAttackOptions *opts);
	int GetSummonedPetCountOnHateList();
	int GetHateRatio(Mob *top, Mob *other);

	int64 GetEntHateAmount(Mob *ent, bool in_damage = false);

	std::list<struct_HateList *> &GetHateList() { return list; }

	std::list<struct_HateList *> GetFilteredHateList(
		EntityFilterType filter_type = EntityFilterType::All,
		uint32 distance = 0
	);

	void DamageHateList(
		int64 damage,
		uint32 distance = 0,
		EntityFilterType filter_type = EntityFilterType::All,
		bool is_percentage = false
	);

	void AddEntToHateList(
		Mob *ent,
		int64 in_hate = 0,
		int64 in_damage = 0,
		bool in_is_frenzied = false,
		bool add_to_hate_list_if_not_exist = true
	);
	void DoFactionHits(int64 npc_faction_level_id, int32 faction_id, int32 faction_value);
	void PrintHateListToClient(Client *c);
	void SetHateAmountOnEnt(Mob *other, int64 in_hate, uint64 in_damage);
	void SetHateOwner(Mob *new_hate_owner) { hate_owner = new_hate_owner; }
	void SpellCast(Mob *caster, uint32 spell_id, float range, Mob *ae_center = nullptr);
	void WipeHateList(bool npc_only = false);
	void RemoveStaleEntries(int time_ms, float dist);


protected:
	struct_HateList* Find(Mob* m);
private:
	std::list<struct_HateList *> list;
	Mob                          *hate_owner;
};
