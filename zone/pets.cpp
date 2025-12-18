/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemu.org)

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

#include "pets.h"

#include "common/repositories/character_pet_name_repository.h"
#include "common/repositories/pets_beastlord_data_repository.h"
#include "common/repositories/pets_repository.h"
#include "common/spdat.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/client.h"
#include "zone/entity.h"
#include "zone/mob.h"
#include "zone/zonedb.h"

#include <string>

// need to pass in a char array of 64 chars
void GetRandPetName(char *name)
{
	std::string temp;
	temp.reserve(64);
	// note these orders are used to make the exclusions cheap :P
	static const char *part1[] = {"G", "J", "K", "L", "V", "X", "Z"};
	static const char *part2[] = {nullptr, "ab", "ar", "as", "eb", "en", "ib", "ob", "on"};
	static const char *part3[] = {nullptr, "an", "ar", "ek", "ob"};
	static const char *part4[] = {"er", "ab", "n", "tik"};

	const char *first = part1[zone->random.Int(0, (sizeof(part1) / sizeof(const char *)) - 1)];
	const char *second = part2[zone->random.Int(0, (sizeof(part2) / sizeof(const char *)) - 1)];
	const char *third = part3[zone->random.Int(0, (sizeof(part3) / sizeof(const char *)) - 1)];
	const char *fourth = part4[zone->random.Int(0, (sizeof(part4) / sizeof(const char *)) - 1)];

	// if both of these are empty, we would get an illegally short name
	if (second == nullptr && third == nullptr)
		fourth = part4[(sizeof(part4) / sizeof(const char *)) - 1];

	// "ektik" isn't allowed either I guess?
	if (third == part3[3] && fourth == part4[3])
		fourth = part4[zone->random.Int(0, (sizeof(part4) / sizeof(const char *)) - 2)];

	// "Laser" isn't allowed either I guess?
	if (first == part1[3] && second == part2[3] && third == nullptr && fourth == part4[0])
		fourth = part4[zone->random.Int(1, (sizeof(part4) / sizeof(const char *)) - 2)];

	temp += first;
	if (second != nullptr)
		temp += second;
	if (third != nullptr)
		temp += third;
	temp += fourth;

	strn0cpy(name, temp.c_str(), 64);
}

void Mob::MakePet(uint16 spell_id, const char* pettype, const char *petname) {
	// petpower of -1 is used to get the petpower based on whichever focus is currently
	// equipped. This should replicate the old functionality for the most part.
	MakePoweredPet(spell_id, pettype, -1, petname);
}

// Split from the basic MakePet to allow backward compatiblity with existing code while also
// making it possible for petpower to be retained without the focus item having to
// stay equipped when the character zones. petpower of -1 means that the currently equipped petfocus
// of a client is searched for and used instead.
void Mob::MakePoweredPet(uint16 spell_id, const char* pettype, int16 petpower,
		const char *petname, float in_size) {
	// Sanity and early out checking first.
	if(HasPet() || pettype == nullptr)
		return;

	int16 act_power = 0; // The actual pet power we'll use.
	if (petpower == -1) {
		if (IsClient()) {
			act_power = CastToClient()->GetFocusEffect(focusPetPower, spell_id);//Client only
		}
		else if (IsBot())
			act_power = CastToBot()->GetFocusEffect(focusPetPower, spell_id);
	}
	else if (petpower > 0)
		act_power = petpower;

	// optional rule: classic style variance in pets. Achieve this by
	// adding a random 0-4 to pet power, since it only comes in increments
	// of five from focus effects.

	//lookup our pets table record for this type
	PetRecord record;
	if(!content_db.GetPoweredPetEntry(pettype, act_power, &record)) {
		Message(Chat::Red, "Unable to find data for pet %s", pettype);
		LogError("Unable to find data for pet [{}], check pets table", pettype);
		return;
	}

	//find the NPC data for the specified NPC type
	const NPCType *base = content_db.LoadNPCTypesData(record.npc_type);
	if(base == nullptr) {
		Message(Chat::Red, "Unable to load NPC data for pet %s", pettype);
		LogError("Unable to load NPC data for pet [{}] (NPC ID [{}]), check pets and npc_types tables", pettype, record.npc_type);
		return;
	}

	//we copy the npc_type data because we need to edit it a bit
	auto npc_type = new NPCType;
	memcpy(npc_type, base, sizeof(NPCType));

	// If pet power is set to -1 in the DB, use stat scaling
	if ((IsClient() || IsBot()) && record.petpower == -1)
	{
		float scale_power = (float)act_power / 100.0f;
		if(scale_power > 0)
		{
			npc_type->max_hp *= (1 + scale_power);
			npc_type->current_hp = npc_type->max_hp;
			npc_type->AC *= (1 + scale_power);
			npc_type->level += 1 + ((int)act_power / 25) > npc_type->level + RuleR(Pets, PetPowerLevelCap) ? RuleR(Pets, PetPowerLevelCap) : 1 + ((int)act_power / 25); // gains an additional level for every 25 pet power
			npc_type->min_dmg = (npc_type->min_dmg * (1 + (scale_power / 2)));
			npc_type->max_dmg = (npc_type->max_dmg * (1 + (scale_power / 2)));
			npc_type->size = npc_type->size * (1 + (scale_power / 2)) > npc_type->size * 3 ? npc_type->size * 3 : npc_type-> size * (1 + (scale_power / 2));
		}
		record.petpower = act_power;
	}

	//Live AA - Elemental Durability
	int64 MaxHP = aabonuses.PetMaxHP + itembonuses.PetMaxHP + spellbonuses.PetMaxHP;

	if (MaxHP){
		npc_type->max_hp += (npc_type->max_hp*MaxHP)/100;
		npc_type->current_hp = npc_type->max_hp;
	}

	//TODO: think about regen (engaged vs. not engaged)

	// Pet naming:
	// 0 - `s pet
	// 1 - `s familiar
	// 2 - `s Warder
	// 3 - Random name if client, `s pet for others
	// 4 - Keep DB name
	// 5 - `s ward

	const auto vanity_name = (IsClient() && !petname) ? CharacterPetNameRepository::FindOne(database, CastToClient()->CharacterID()) : CharacterPetNameRepository::CharacterPetName{};

	if (
		IsClient() &&
		!petname &&
		!vanity_name.name.empty()
	) {
		petname = vanity_name.name.c_str();
	}

	if (petname != nullptr) {
		// Name was provided, use it.
		strn0cpy(npc_type->name, petname, 64);
		EntityList::RemoveNumbers(npc_type->name);
		entity_list.MakeNameUnique(npc_type->name);
	} else if (record.petnaming == 0) {
		strcpy(npc_type->name, GetCleanName());
		npc_type->name[25] = '\0';
		strcat(npc_type->name, "`s_pet");
	} else if (record.petnaming == 1) {
		strcpy(npc_type->name, GetName());
		npc_type->name[19] = '\0';
		strcat(npc_type->name, "`s_familiar");
	} else if (record.petnaming == 2) {
		strcpy(npc_type->name, GetName());
		npc_type->name[21] = 0;
		strcat(npc_type->name, "`s_Warder");
	} else if (record.petnaming == 4) {
		// Keep the DB name
	} else if (record.petnaming == 3 && IsClient()) {
		GetRandPetName(npc_type->name);
	} else if (record.petnaming == 5 && IsClient()) {
		strcpy(npc_type->name, GetName());
		npc_type->name[24] = '\0';
		strcat(npc_type->name, "`s_ward");
	} else {
		strcpy(npc_type->name, GetCleanName());
		npc_type->name[25] = '\0';
		strcat(npc_type->name, "`s_pet");
	}

	// Beastlord Pets
	if (record.petnaming == 2) {
		uint16 race_id = GetBaseRace();

		auto d = content_db.GetBeastlordPetData(race_id);

		npc_type->race        = d.race_id;
		npc_type->texture     = d.texture;
		npc_type->helmtexture = d.helm_texture;
		npc_type->gender      = d.gender;
		npc_type->luclinface  = d.face;

		npc_type->size *= d.size_modifier;
	}

	// handle monster summoning pet appearance
	if(record.monsterflag) {

		uint32 monsterid = 0;

		// get a random npc id from the spawngroups assigned to this zone
		auto query = StringFormat("SELECT npcID "
									"FROM (spawnentry INNER JOIN spawn2 ON spawn2.spawngroupID = spawnentry.spawngroupID) "
									"INNER JOIN npc_types ON npc_types.id = spawnentry.npcID "
									"WHERE spawn2.zone = '%s' AND npc_types.bodytype NOT IN (11, 33, 66, 67) "
									"AND npc_types.race NOT IN (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 44, "
									"55, 67, 71, 72, 73, 77, 78, 81, 90, 92, 93, 94, 106, 112, 114, 127, 128, "
									"130, 139, 141, 183, 236, 237, 238, 239, 254, 266, 329, 330, 378, 379, "
									"380, 381, 382, 383, 404, 522) "
									"ORDER BY RAND() LIMIT 1", zone->GetShortName());
		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			safe_delete(npc_type);
			return;
		}

		if (results.RowCount() != 0) {
			auto row = results.begin();
			monsterid = Strings::ToInt(row[0]);
		}

		// since we don't have any monsters, just make it look like an earth pet for now
		if (monsterid == 0)
			monsterid = 567;

		// give the summoned pet the attributes of the monster we found
		const NPCType* monster = content_db.LoadNPCTypesData(monsterid);
		if(monster) {
			npc_type->race = monster->race;
			npc_type->size = monster->size;
			npc_type->texture = monster->texture;
			npc_type->gender = monster->gender;
			npc_type->luclinface = monster->luclinface;
			npc_type->helmtexture = monster->helmtexture;
			npc_type->herosforgemodel = monster->herosforgemodel;
		} else
			LogError("Error loading NPC data for monster summoning pet (NPC ID [{}])", monsterid);

	}

	//this takes ownership of the npc_type data
	auto npc = new Pet(npc_type, this, record.petcontrol, spell_id, record.petpower);

	// Now that we have an actual object to interact with, load
	// the base items for the pet. These are always loaded
	// so that a rank 1 suspend minion does not kill things
	// like the special back items some focused pets may receive.
	uint32 petinv[EQ::invslot::EQUIPMENT_COUNT];
	memset(petinv, 0, sizeof(petinv));
	const EQ::ItemData *item = nullptr;

	if (content_db.GetBasePetItems(record.equipmentset, petinv)) {
		for (int i = EQ::invslot::EQUIPMENT_BEGIN; i <= EQ::invslot::EQUIPMENT_END; i++)
			if (petinv[i]) {
				item = database.GetItem(petinv[i]);
				npc->AddLootDrop(item, LootdropEntriesRepository::NewNpcEntity(), true);
			}
	}

	npc->UpdateEquipmentLight();

	// finally, override size if one was provided
	if (in_size > 0.0f)
		npc->size = in_size;

	entity_list.AddNPC(npc, true, true);
	SetPetID(npc->GetID());
	// We need to handle PetType 5 (petHatelist), add the current target to the hatelist of the pet

	if (record.petcontrol == PetType::TargetLock)
	{
		Mob* m_target = GetTarget();

		bool activiate_pet = false;
		if (m_target && m_target->GetID() != GetID()) {

			if (spells[spell_id].target_type == ST_Self) {
				float distance = CalculateDistance(m_target->GetX(), m_target->GetY(), m_target->GetZ());
				if (distance <= 200) { //Live distance on targetlock pets that self cast. No message is given if not in range.
					activiate_pet = true;
				}
			}
			else {
				activiate_pet = true;
			}
		}

		if (activiate_pet){
			npc->AddToHateList(m_target, 1);
			npc->SetPetTargetLockID(m_target->GetID());
			npc->SetSpecialAbility(SpecialAbility::AggroImmunity, 1);
		}
		else {
			npc->CastSpell(SPELL_UNSUMMON_SELF, npc->GetID()); //Live like behavior, damages self for 20K
			if (!npc->HasDied()) {
				npc->Kill(); //Ensure pet dies if over 20k HP.
			}
		}
	}
}

void NPC::TryDepopTargetLockedPets(Mob* current_target) {

	if (!current_target || (current_target && (current_target->GetID() != GetPetTargetLockID()) || current_target->IsCorpse())) {

		//Use when swarmpets are set to auto lock from quest or rule
		if (GetSwarmInfo() && GetSwarmInfo()->target) {
			Mob* owner = entity_list.GetMobID(GetSwarmInfo()->owner_id);
			if (owner) {
				owner->SetTempPetCount(owner->GetTempPetCount() - 1);
			}
			Depop();
			return;
		}
		//Use when pets are given petype 5
		if (IsPet() && GetPetType() == PetType::TargetLock && GetPetTargetLockID()) {
			CastSpell(SPELL_UNSUMMON_SELF, GetID()); //Live like behavior, damages self for 20K
			if (!HasDied()) {
				Kill(); //Ensure pet dies if over 20k HP.
			}
			return;
		}
	}
}



/* This is why the pets ghost - pets were being spawned too far away from its npc owner and some
into walls or objects (+10), this sometimes creates the "ghost" effect. I changed to +2 (as close as I
could get while it still looked good). I also noticed this can happen if an NPC is spawned on the same spot of another or in a related bad spot.*/
Pet::Pet(NPCType *type_data, Mob *owner, uint8 pet_type, uint16 spell_id, int16 power)
: NPC(type_data, 0, owner->GetPosition() + glm::vec4(2.0f, 2.0f, 0.0f, 0.0f), GravityBehavior::Water)
{
	GiveNPCTypeData(type_data);
	SetPetType(pet_type);
	SetPetPower(power);
	SetOwnerID(owner ? owner->GetID() : 0);
	SetPetSpellID(spell_id);

	// All pets start at false on newer clients. The client
	// turns it on and tracks the state.
	SetTaunting(false);

	// Older clients didn't track state, and default taunting is on (per @mackal)
	// Familiar and animation pets don't get taunt until an AA.
	if (owner && owner->IsClient()) {
		if (!(owner->CastToClient()->ClientVersionBit() & EQ::versions::maskUFAndLater)) {
			if (
				(GetPetType() != PetType::Familiar && GetPetType() != PetType::Animation) ||
				aabonuses.PetCommands[PetCommand::Taunt]
			) {
				SetTaunting(true);
			}
		}
	}

	// Class should use npc constructor to set light properties
}

bool ZoneDatabase::GetPetEntry(const std::string& pet_type, PetRecord *p)
{
	return GetPoweredPetEntry(pet_type, 0, p);
}

bool ZoneDatabase::GetPoweredPetEntry(const std::string& pet_type, int16 pet_power, PetRecord* r)
{
	const auto& l = PetsRepository::GetWhere(
		content_db,
		fmt::format(
			"`type` = '{}' AND `petpower` <= {} ORDER BY `petpower` DESC LIMIT 1",
			pet_type,
			pet_power <= 0 ? 0 : pet_power
		)
	);

	if (l.empty()) {
		return false;
	}

	auto &e = l.front();

	r->npc_type     = e.npcID;
	r->temporary    = e.temp;
	r->petpower     = e.petpower;
	r->petcontrol   = e.petcontrol;
	r->petnaming    = e.petnaming;
	r->monsterflag  = e.monsterflag;
	r->equipmentset = e.equipmentset;

	return true;
}

Mob* Mob::GetPet() {
	if (!GetPetID()) {
		return nullptr;
	}

	const auto m = entity_list.GetMob(GetPetID());
	if (!m) {
		SetPetID(0);
		return nullptr;
	}

	if (m->GetOwnerID() != GetID()) {
		SetPetID(0);
		return nullptr;
	}

	return m;
}

bool Mob::HasPet() const {
	if (GetPetID() == 0) {
		return false;
	}

	const auto m = entity_list.GetMob(GetPetID());
	if (!m) {
		return false;
	}

	if (m->GetOwnerID() != GetID()) {
		return false;
	}

	return true;
}

void Mob::SetPet(Mob* newpet) {
	Mob* oldpet = GetPet();
	if (oldpet) {
		oldpet->SetOwnerID(0);
	}
	if (newpet == nullptr) {
		SetPetID(0);
	} else {
		SetPetID(newpet->GetID());
		Mob* oldowner = entity_list.GetMob(newpet->GetOwnerID());
		if (oldowner)
			oldowner->SetPetID(0);
		newpet->SetOwnerID(GetID());
	}
}

void Mob::SetPetID(uint16 NewPetID) {
	if (NewPetID == GetID() && NewPetID != 0)
		return;
	petid = NewPetID;

	if(IsClient())
	{
		Mob* NewPet = entity_list.GetMob(NewPetID);
		CastToClient()->UpdateXTargetType(MyPet, NewPet);
	}
}

void NPC::GetPetState(SpellBuff_Struct *pet_buffs, uint32 *items, char *name) {
	//save the pet name
	strn0cpy(name, GetName(), 64);

	//save their items, we only care about what they are actually wearing
	memcpy(items, equipment, sizeof(uint32) * EQ::invslot::EQUIPMENT_COUNT);

	//save their buffs.
	for (int i=EQ::invslot::EQUIPMENT_BEGIN; i < GetPetMaxTotalSlots(); i++) {
		if (IsValidSpell(buffs[i].spellid)) {
			pet_buffs[i].spellid = buffs[i].spellid;
			pet_buffs[i].effect_type = i+1;
			pet_buffs[i].duration = buffs[i].ticsremaining;
			pet_buffs[i].level = buffs[i].casterlevel;
			pet_buffs[i].bard_modifier = 10;
			pet_buffs[i].counters = buffs[i].counters;
			pet_buffs[i].bard_modifier = buffs[i].instrument_mod;
		}
		else {
			pet_buffs[i].spellid = SPELL_UNKNOWN;
			pet_buffs[i].duration = 0;
			pet_buffs[i].level = 0;
			pet_buffs[i].bard_modifier = 10;
			pet_buffs[i].counters = 0;
		}
	}
}

void NPC::SetPetState(SpellBuff_Struct *pet_buffs, uint32 *items) {
	//restore their buffs...

	int i;
	for (i = 0; i < GetPetMaxTotalSlots(); i++) {
		for(int z = 0; z < GetPetMaxTotalSlots(); z++) {
		// check for duplicates
			if(IsValidSpell(buffs[z].spellid) && buffs[z].spellid == pet_buffs[i].spellid) {
				buffs[z].spellid = SPELL_UNKNOWN;
				pet_buffs[i].spellid = 0xFFFFFFFF;
			}
		}

		if (pet_buffs[i].spellid <= (uint32)SPDAT_RECORDS && pet_buffs[i].spellid != 0 && (pet_buffs[i].duration > 0 || pet_buffs[i].duration == -1)) {
			if(pet_buffs[i].level == 0 || pet_buffs[i].level > 100)
				pet_buffs[i].level = 1;
			buffs[i].spellid			= pet_buffs[i].spellid;
			buffs[i].ticsremaining		= pet_buffs[i].duration;
			buffs[i].casterlevel		= pet_buffs[i].level;
			buffs[i].casterid			= 0;
			buffs[i].counters			= pet_buffs[i].counters;
			buffs[i].hit_number			= spells[pet_buffs[i].spellid].hit_number;
			buffs[i].instrument_mod		= pet_buffs[i].bard_modifier;
		}
		else {
			buffs[i].spellid = SPELL_UNKNOWN;
			pet_buffs[i].spellid = 0xFFFFFFFF;
			pet_buffs[i].effect_type = 0;
			pet_buffs[i].level = 0;
			pet_buffs[i].duration = 0;
			pet_buffs[i].bard_modifier = 0;
		}
	}
	for (int j1=0; j1 < GetPetMaxTotalSlots(); j1++) {
		if (buffs[j1].spellid <= (uint32)SPDAT_RECORDS) {
			for (int x1=0; x1 < EFFECT_COUNT; x1++) {
				switch (spells[buffs[j1].spellid].effect_id[x1]) {
					case SpellEffect::AddMeleeProc:
					case SpellEffect::WeaponProc:
						// We need to reapply buff based procs
						// We need to do this here so suspended pets also regain their procs.
						AddProcToWeapon(GetProcID(buffs[j1].spellid,x1), false, 100+spells[buffs[j1].spellid].limit_value[x1], buffs[j1].spellid, buffs[j1].casterlevel, GetSpellProcLimitTimer(buffs[j1].spellid, ProcType::MELEE_PROC));
						break;
					case SpellEffect::DefensiveProc:
						AddDefensiveProc(GetProcID(buffs[j1].spellid, x1), 100 + spells[buffs[j1].spellid].limit_value[x1], buffs[j1].spellid, GetSpellProcLimitTimer(buffs[j1].spellid, ProcType::DEFENSIVE_PROC));
						break;
					case SpellEffect::RangedProc:
						AddRangedProc(GetProcID(buffs[j1].spellid, x1), 100 + spells[buffs[j1].spellid].limit_value[x1], buffs[j1].spellid, GetSpellProcLimitTimer(buffs[j1].spellid, ProcType::RANGED_PROC));
						break;
					case SpellEffect::Charm:
					case SpellEffect::Rune:
					case SpellEffect::NegateAttacks:
					case SpellEffect::Illusion:
						buffs[j1].spellid = SPELL_UNKNOWN;
						pet_buffs[j1].spellid = SPELLBOOK_UNKNOWN;
						pet_buffs[j1].effect_type = 0;
						pet_buffs[j1].level = 0;
						pet_buffs[j1].duration = 0;
						pet_buffs[j1].bard_modifier = 0;
						x1 = EFFECT_COUNT;
						break;
					// We can't send appearance packets yet, put down at CompleteConnect
				}
			}
		}
	}

	//restore their equipment...
	for (i = EQ::invslot::EQUIPMENT_BEGIN; i <= EQ::invslot::EQUIPMENT_END; i++) {
		if (items[i] == 0) {
			continue;
		}

		const EQ::ItemData *item2 = database.GetItem(items[i]);

		if (item2) {
			bool noDrop           = (item2->NoDrop == 0); // Field is reverse logic
			bool petCanHaveNoDrop = (RuleB(Pets, CanTakeNoDrop) && _CLIENTPET(this) && GetPetType() <= PetType::Normal);

			if (!noDrop || petCanHaveNoDrop) {
				AddLootDrop(item2, LootdropEntriesRepository::NewNpcEntity(), true);
			}
		}
	}
}

// Load the equipmentset from the DB. Might be worthwhile to load these into
// shared memory at some point due to the number of queries needed to load a
// nested set.
bool ZoneDatabase::GetBasePetItems(int32 equipmentset, uint32 *items) {
	if (equipmentset < 0 || items == nullptr)
		return false;

	// Equipment sets can be nested. We start with the top-most one and
	// add all items in it to the items array. Referenced equipmentsets
	// are loaded after that, up to a max depth of 5. (Arbitrary limit
	// so we don't go into an endless loop if the DB data is cyclic for
	// some reason.)
	// A slot will only get an item put in it if it is empty. That way
	// an equipmentset can overload a slot for the set(s) it includes.

	int depth = 0;
	int32 curset = equipmentset;
	int32 nextset = -1;
	uint32 slot;

	// outline:
	// get equipmentset from DB. (Mainly check if we exist and get the
	// nested ID)
	// query pets_equipmentset_entries with the set_id and loop over
	// all of the result rows. Check if we have something in the slot
	// already. If no, add the item id to the equipment array.
	while (curset >= 0 && depth < 5) {
		std::string  query = StringFormat("SELECT nested_set FROM pets_equipmentset WHERE set_id = '%d'", curset);
		auto results = QueryDatabase(query);
		if (!results.Success()) {
			return false;
		}

		if (results.RowCount() != 1) {
			// invalid set reference, it doesn't exist
			LogError("Error in GetBasePetItems equipment set [{}] does not exist", curset);
			return false;
		}

		auto row = results.begin();
		nextset = Strings::ToInt(row[0]);

		query = StringFormat("SELECT slot, item_id FROM pets_equipmentset_entries WHERE set_id='%d'", curset);
		results = QueryDatabase(query);
		if (results.Success()) {
			for (row = results.begin(); row != results.end(); ++row)
			{
				slot = Strings::ToInt(row[0]);

				if (slot > EQ::invslot::EQUIPMENT_END)
					continue;

				if (items[slot] == 0)
					items[slot] = Strings::ToInt(row[1]);
			}
		}

		curset = nextset;
		depth++;
	}

	return true;
}

bool Pet::CheckSpellLevelRestriction(Mob *caster, uint16 spell_id)
{
	auto owner = GetOwner();
	if (owner)
		return owner->CheckSpellLevelRestriction(caster, spell_id);
	return true;
}

BeastlordPetData::PetStruct ZoneDatabase::GetBeastlordPetData(uint16 race_id) {
	BeastlordPetData::PetStruct d;

	const auto& e = PetsBeastlordDataRepository::FindOne(*this, race_id);

	if (!e.player_race) {
		return d;
	}

	d.race_id       = e.pet_race;
	d.texture       = e.texture;
	d.helm_texture  = e.helm_texture;
	d.gender        = e.gender;
	d.size_modifier = e.size_modifier;
	d.face          = e.face;

	return d;
}
