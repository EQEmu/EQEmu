/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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

#include "aa.h"

#include "common/classes.h"
#include "common/eq_packet_structs.h"
#include "common/eqemu_logsys.h"
#include "common/events/player_event_logs.h"
#include "common/races.h"
#include "common/repositories/aa_ability_repository.h"
#include "common/repositories/aa_rank_effects_repository.h"
#include "common/repositories/aa_rank_prereqs_repository.h"
#include "common/repositories/aa_ranks_repository.h"
#include "common/repositories/character_alternate_abilities_repository.h"
#include "common/spdat.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/client.h"
#include "zone/corpse.h"
#include "zone/groups.h"
#include "zone/mob.h"
#include "zone/queryserv.h"
#include "zone/quest_parser_collection.h"
#include "zone/raids.h"
#include "zone/string_ids.h"
#include "zone/titles.h"
#include "zone/worldserver.h"
#include "zone/zonedb.h"

extern WorldServer worldserver;
extern QueryServ* QServ;

void Mob::TemporaryPets(uint16 spell_id, Mob *targ, const char *name_override, uint32 duration_override, bool followme, bool sticktarg, uint16 *eye_id) {

	//It might not be a bad idea to put these into the database, eventually..

	//Dook- swarms and wards

	// do nothing if it's a corpse
	if (targ != nullptr && targ->IsCorpse())
		return;

	// yep, even these need pet power!
	int act_power = 0;

	if (IsOfClientBot()) {
		act_power = GetFocusEffect(focusPetPower, spell_id);
		if (IsClient()) {
			act_power = CastToClient()->GetFocusEffect(focusPetPower, spell_id);
		}
	}

	PetRecord record;
	if (!content_db.GetPoweredPetEntry(spells[spell_id].teleport_zone, act_power, &record))
	{
		LogError("Unknown swarm pet spell id: {}, check pets table", spell_id);
		Message(Chat::Red, "Unable to find data for pet %s", spells[spell_id].teleport_zone);
		return;
	}

	SwarmPet_Struct pet;
	pet.count = 1;
	pet.duration = 1;

	for (int x = 0; x < MAX_SWARM_PETS; x++)
	{
		if (spells[spell_id].effect_id[x] == SpellEffect::TemporaryPets)
		{
			pet.count = spells[spell_id].base_value[x];
			pet.duration = spells[spell_id].max_value[x];
		}
	}

	pet.duration += GetFocusEffect(focusSwarmPetDuration, spell_id) / 1000;

	pet.npc_id = record.npc_type;

	NPCType *made_npc = nullptr;

	const NPCType *npc_type = content_db.LoadNPCTypesData(pet.npc_id);
	if (npc_type == nullptr) {
		//log write
		LogError("Unknown npc type for swarm pet spell id: [{}]", spell_id);
		Message(0, "Unable to find pet!");
		return;
	}

	if (name_override != nullptr) {
		//we have to make a custom NPC type for this name change
		made_npc = new NPCType;
		memcpy(made_npc, npc_type, sizeof(NPCType));
		strcpy(made_npc->name, name_override);
		npc_type = made_npc;
	}

	int summon_count = 0;
	summon_count = pet.count;

	if (summon_count > MAX_SWARM_PETS)
		summon_count = MAX_SWARM_PETS;

	static const glm::vec2 swarmPetLocations[MAX_SWARM_PETS] = {
		glm::vec2(5, 5), glm::vec2(-5, 5), glm::vec2(5, -5), glm::vec2(-5, -5),
		glm::vec2(10, 10), glm::vec2(-10, 10), glm::vec2(10, -10), glm::vec2(-10, -10),
		glm::vec2(8, 8), glm::vec2(-8, 8), glm::vec2(8, -8), glm::vec2(-8, -8)
	};

	NPC* swarm_pet_npc = nullptr;

	while (summon_count > 0) {
		int pet_duration = pet.duration;
		if (duration_override > 0)
			pet_duration = duration_override;

		//this is a little messy, but the only way to do it right
		//it would be possible to optimize out this copy for the last pet, but oh well
		NPCType *npc_dup = nullptr;
		if (made_npc != nullptr) {
			npc_dup = new NPCType;
			memcpy(npc_dup, made_npc, sizeof(NPCType));
		}

		swarm_pet_npc = new NPC(
			(npc_dup != nullptr) ? npc_dup : npc_type,	//make sure we give the NPC the correct data pointer
			0,
			GetPosition() + glm::vec4(swarmPetLocations[summon_count], 0.0f, 0.0f),
			GravityBehavior::Water);

		if (followme)
			swarm_pet_npc->SetFollowID(GetID());

		if (!swarm_pet_npc->GetSwarmInfo()) {
			auto nSI = new SwarmPet;
			swarm_pet_npc->SetSwarmInfo(nSI);
			swarm_pet_npc->GetSwarmInfo()->duration = new Timer(pet_duration * 1000);
		}
		else {
			swarm_pet_npc->GetSwarmInfo()->duration->Start(pet_duration * 1000);
		}

		swarm_pet_npc->StartSwarmTimer(pet_duration * 1000);

		//removing this prevents the pet from attacking
		swarm_pet_npc->GetSwarmInfo()->owner_id = GetID();

		//give the pets somebody to "love"
		if (targ != nullptr) {
			swarm_pet_npc->AddToHateList(targ, 1000, 1000);
			if (RuleB(Spells, SwarmPetTargetLock) || sticktarg) {
				swarm_pet_npc->GetSwarmInfo()->target = targ->GetID();
				swarm_pet_npc->SetPetTargetLockID(targ->GetID());
				swarm_pet_npc->SetSpecialAbility(SpecialAbility::AggroImmunity, 1);
			}
			else {
				swarm_pet_npc->GetSwarmInfo()->target = 0;
			}
		}

		//we allocated a new NPC type object, give the NPC ownership of that memory
		if (npc_dup != nullptr)
			swarm_pet_npc->GiveNPCTypeData(npc_dup);

		entity_list.AddNPC(swarm_pet_npc, true, true);
		summon_count--;
	}

	if (swarm_pet_npc && IsClient() && eye_id != nullptr) {
		*eye_id = swarm_pet_npc->GetID();
	}

	//the target of these swarm pets will take offense to being cast on...
	if (targ != nullptr)
		targ->AddToHateList(this, 1, 0);

	// The other pointers we make are handled elsewhere.
	delete made_npc;
}

void Mob::TypesTemporaryPets(uint32 typesid, Mob *targ, const char *name_override, uint32 duration_override, bool followme, bool sticktarg) {

	SwarmPet_Struct pet;
	pet.count = 1;
	pet.duration = 1;

	pet.npc_id = typesid;

	NPCType *made_npc = nullptr;

	const NPCType *npc_type = content_db.LoadNPCTypesData(typesid);
	if(npc_type == nullptr) {
		//log write
		LogError("Unknown npc type for swarm pet type id: [{}]", typesid);
		Message(0,"Unable to find pet!");
		return;
	}

	if(name_override != nullptr) {
		//we have to make a custom NPC type for this name change
		made_npc = new NPCType;
		memcpy(made_npc, npc_type, sizeof(NPCType));
		strcpy(made_npc->name, name_override);
		npc_type = made_npc;
	}

	int summon_count = 0;
	summon_count = pet.count;

	static const glm::vec2 swarmPetLocations[MAX_SWARM_PETS] = {
		glm::vec2(5, 5), glm::vec2(-5, 5), glm::vec2(5, -5), glm::vec2(-5, -5),
		glm::vec2(10, 10), glm::vec2(-10, 10), glm::vec2(10, -10), glm::vec2(-10, -10),
		glm::vec2(8, 8), glm::vec2(-8, 8), glm::vec2(8, -8), glm::vec2(-8, -8)
	};

	while(summon_count > 0) {
		int pet_duration = pet.duration;
		if(duration_override > 0)
			pet_duration = duration_override;

		//this is a little messy, but the only way to do it right
		//it would be possible to optimize out this copy for the last pet, but oh well
		NPCType *npc_dup = nullptr;
		if(made_npc != nullptr) {
			npc_dup = new NPCType;
			memcpy(npc_dup, made_npc, sizeof(NPCType));
		}

		NPC* swarm_pet_npc = new NPC(
				(npc_dup!=nullptr)?npc_dup:npc_type,	//make sure we give the NPC the correct data pointer
				0,
				GetPosition() + glm::vec4(swarmPetLocations[summon_count], 0.0f, 0.0f),
				GravityBehavior::Water);

		if (followme)
			swarm_pet_npc->SetFollowID(GetID());

		if(!swarm_pet_npc->GetSwarmInfo()){
			auto nSI = new SwarmPet;
			swarm_pet_npc->SetSwarmInfo(nSI);
			swarm_pet_npc->GetSwarmInfo()->duration = new Timer(pet_duration*1000);
		}
		else {
			swarm_pet_npc->GetSwarmInfo()->duration->Start(pet_duration*1000);
		}

		swarm_pet_npc->StartSwarmTimer(pet_duration * 1000);

		//removing this prevents the pet from attacking
		swarm_pet_npc->GetSwarmInfo()->owner_id = GetID();

		//give the pets somebody to "love"
		if(targ != nullptr){
			swarm_pet_npc->AddToHateList(targ, 1000, 1000);

			if (RuleB(Spells, SwarmPetTargetLock) || sticktarg) {
				swarm_pet_npc->GetSwarmInfo()->target = targ->GetID();
				swarm_pet_npc->SetPetTargetLockID(targ->GetID());
				swarm_pet_npc->SetSpecialAbility(SpecialAbility::AggroImmunity, 1);
			}
			else {
				swarm_pet_npc->GetSwarmInfo()->target = 0;
			}
		}

		//we allocated a new NPC type object, give the NPC ownership of that memory
		if(npc_dup != nullptr)
			swarm_pet_npc->GiveNPCTypeData(npc_dup);

		entity_list.AddNPC(swarm_pet_npc, true, true);
		summon_count--;
	}

	// The other pointers we make are handled elsewhere.
	delete made_npc;
}

void Mob::WakeTheDead(uint16 spell_id, Corpse *corpse_to_use, Mob *tar, uint32 duration) {

	/*
		SPA 299 Wake The Dead, 'animateDead' should be temp pet, always spawns 1 pet from corpse, max value is duration
		SPA 306 Wake The Dead, 'animateDead#' should be temp pet, base is amount of pets from indivual corpses, max value is duration
		Max range for closet corpse is 250 units.
		TODO: Should use temp pets
	*/

	if (!corpse_to_use) {
		return;
	}

	SwarmPet_Struct pet;
	pet.count = 1;
	pet.duration = 1;

	//pet.duration += GetFocusEffect(focusSwarmPetDuration, spell_id) / 1000; //TODO: Does WTD use pet focus?

	pet.npc_id = WAKE_THE_DEAD_NPCTYPEID;

	NPCType *made_npc = nullptr;

	const NPCType *npc_type = content_db.LoadNPCTypesData(WAKE_THE_DEAD_NPCTYPEID);
	if (npc_type == nullptr) {
		//log write
		LogError("Unknown npc type for 'Wake the Dead' swarm pet spell id: [{}]", spell_id);
		Message(0, "Unable to find pet!");
		return;
	}

	made_npc = new NPCType;
	memcpy(made_npc, npc_type, sizeof(NPCType));

	char NewName[64];
	sprintf(NewName, "%s`s Animated Corpse", GetCleanName());
	strcpy(made_npc->name, NewName);
	npc_type = made_npc;

	//combat stats
	made_npc->AC = ((GetLevel() * 7) + 550);
	made_npc->ATK = GetLevel();
	made_npc->max_dmg = (GetLevel() * 4) + 2;
	made_npc->min_dmg = 1;

	//base stats
	made_npc->current_hp = (GetLevel() * 55);
	made_npc->max_hp = (GetLevel() * 55);
	made_npc->STR = 85 + (GetLevel() * 3);
	made_npc->STA = 85 + (GetLevel() * 3);
	made_npc->DEX = 85 + (GetLevel() * 3);
	made_npc->AGI = 85 + (GetLevel() * 3);
	made_npc->INT = 85 + (GetLevel() * 3);
	made_npc->WIS = 85 + (GetLevel() * 3);
	made_npc->CHA = 85 + (GetLevel() * 3);
	made_npc->MR = 25;
	made_npc->FR = 25;
	made_npc->CR = 25;
	made_npc->DR = 25;
	made_npc->PR = 25;

	//level class and gender
	made_npc->level = GetLevel();
	made_npc->class_ = corpse_to_use->class_;
	made_npc->race = corpse_to_use->race;
	made_npc->gender = corpse_to_use->gender;
	made_npc->loottable_id = 0;

	//appearance
	made_npc->beard = corpse_to_use->beard;
	made_npc->beardcolor = corpse_to_use->beardcolor;
	made_npc->eyecolor1 = corpse_to_use->eyecolor1;
	made_npc->eyecolor2 = corpse_to_use->eyecolor2;
	made_npc->haircolor = corpse_to_use->haircolor;
	made_npc->hairstyle = corpse_to_use->hairstyle;
	made_npc->helmtexture = corpse_to_use->helmtexture;
	made_npc->luclinface = corpse_to_use->luclinface;
	made_npc->size = corpse_to_use->size;
	made_npc->texture = corpse_to_use->texture;

	//cast stuff.. based off of PEQ's if you want to change
	//it you'll have to mod this code, but most likely
	//most people will be using PEQ style for the first
	//part of their spell list; can't think of any smooth
	//way to do this
	//some basic combat mods here too since it's convienent
	switch (corpse_to_use->class_)
	{
	case Class::Cleric:
		made_npc->npc_spells_id = 1;
		break;
	case Class::Wizard:
		made_npc->npc_spells_id = 2;
		break;
	case Class::Necromancer:
		made_npc->npc_spells_id = 3;
		break;
	case Class::Magician:
		made_npc->npc_spells_id = 4;
		break;
	case Class::Enchanter:
		made_npc->npc_spells_id = 5;
		break;
	case Class::Shaman:
		made_npc->npc_spells_id = 6;
		break;
	case Class::Druid:
		made_npc->npc_spells_id = 7;
		break;
	case Class::Paladin:
		//SpecialAbility::TripleAttack
		strcpy(made_npc->special_abilities, "6,1");
		made_npc->current_hp = made_npc->current_hp * 150 / 100;
		made_npc->max_hp = made_npc->max_hp * 150 / 100;
		made_npc->npc_spells_id = 8;
		break;
	case Class::ShadowKnight:
		strcpy(made_npc->special_abilities, "6,1");
		made_npc->current_hp = made_npc->current_hp * 150 / 100;
		made_npc->max_hp = made_npc->max_hp * 150 / 100;
		made_npc->npc_spells_id = 9;
		break;
	case Class::Ranger:
		strcpy(made_npc->special_abilities, "7,1");
		made_npc->current_hp = made_npc->current_hp * 135 / 100;
		made_npc->max_hp = made_npc->max_hp * 135 / 100;
		made_npc->npc_spells_id = 10;
		break;
	case Class::Bard:
		strcpy(made_npc->special_abilities, "6,1");
		made_npc->current_hp = made_npc->current_hp * 110 / 100;
		made_npc->max_hp = made_npc->max_hp * 110 / 100;
		made_npc->npc_spells_id = 11;
		break;
	case Class::Beastlord:
		strcpy(made_npc->special_abilities, "7,1");
		made_npc->current_hp = made_npc->current_hp * 110 / 100;
		made_npc->max_hp = made_npc->max_hp * 110 / 100;
		made_npc->npc_spells_id = 12;
		break;
	case Class::Rogue:
		strcpy(made_npc->special_abilities, "7,1");
		made_npc->max_dmg = made_npc->max_dmg * 150 / 100;
		made_npc->current_hp = made_npc->current_hp * 110 / 100;
		made_npc->max_hp = made_npc->max_hp * 110 / 100;
		break;
	case Class::Monk:
		strcpy(made_npc->special_abilities, "7,1");
		made_npc->max_dmg = made_npc->max_dmg * 150 / 100;
		made_npc->current_hp = made_npc->current_hp * 135 / 100;
		made_npc->max_hp = made_npc->max_hp * 135 / 100;
		break;
	case Class::Warrior:
	case Class::Berserker:
		strcpy(made_npc->special_abilities, "7,1");
		made_npc->max_dmg = made_npc->max_dmg * 150 / 100;
		made_npc->current_hp = made_npc->current_hp * 175 / 100;
		made_npc->max_hp = made_npc->max_hp * 175 / 100;
		break;
	default:
		made_npc->npc_spells_id = 0;
		break;
	}

	made_npc->merchanttype = 0;
	made_npc->d_melee_texture1 = 0;
	made_npc->d_melee_texture2 = 0;


	int summon_count = 0;
	summon_count = pet.count;

	NPC* swarm_pet_npc = nullptr;
	//TODO: potenitally add support for multiple pets per corpse
	while (summon_count > 0) {
		int pet_duration = duration;

		NPCType *npc_dup = nullptr;
		if (made_npc != nullptr) {
			npc_dup = new NPCType;
			memcpy(npc_dup, made_npc, sizeof(NPCType));
		}

		swarm_pet_npc = new NPC(
			(npc_dup != nullptr) ? npc_dup : npc_type,
			0, corpse_to_use->GetPosition(),GravityBehavior::Water);

		swarm_pet_npc->SetFollowID(GetID());

		if (!swarm_pet_npc->GetSwarmInfo()) {
			auto nSI = new SwarmPet;
			swarm_pet_npc->SetSwarmInfo(nSI);
			swarm_pet_npc->GetSwarmInfo()->duration = new Timer(pet_duration * 1000);
		}
		else {
			swarm_pet_npc->GetSwarmInfo()->duration->Start(pet_duration * 1000);
		}

		swarm_pet_npc->StartSwarmTimer(pet_duration * 1000);

		//removing this prevents the pet from attacking
		swarm_pet_npc->GetSwarmInfo()->owner_id = GetID();

		//give the pets somebody to "love"
		if (tar != nullptr) {
			swarm_pet_npc->AddToHateList(tar, 10000, 1000);
			swarm_pet_npc->GetSwarmInfo()->target = 0;
		}

		//we allocated a new NPC type object, give the NPC ownership of that memory
		if (npc_dup != nullptr)
			swarm_pet_npc->GiveNPCTypeData(npc_dup);

		entity_list.AddNPC(swarm_pet_npc, true, true);
		summon_count--;
	}

	//the target of these swarm pets will take offense to being cast on...
	if (tar != nullptr)
		tar->AddToHateList(this, 1, 0);

	// The other pointers we make are handled elsewhere.
	delete made_npc;
}

void Client::ResetAA()
{
	SendClearPlayerAA();
	RefundAA();

	memset(&m_pp.aa_array[0], 0, sizeof(AA_Array) * MAX_PP_AA_ARRAY);

	int slot_id = 0;

	for (auto& rank_value: aa_ranks) {
		auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(rank_value.first, rank_value.second.first);
		auto ability      = ability_rank.first;
		auto rank         = ability_rank.second;

		if (!rank) {
			continue;
		}

		m_pp.aa_array[slot_id].AA      = rank_value.first;
		m_pp.aa_array[slot_id].value   = rank_value.second.first;
		m_pp.aa_array[slot_id].charges = rank_value.second.second;
		++slot_id;
	}

	database.DeleteCharacterAAs(CharacterID());
}

void Client::ResetLeadershipAA()
{
	SendClearLeadershipAA();

	for (int slot_id = 0; slot_id < _maxLeaderAA; ++slot_id) {
		m_pp.leader_abilities.ranks[slot_id] = 0;
	}

	m_pp.group_leadership_points = 0;
	m_pp.raid_leadership_points  = 0;
	m_pp.group_leadership_exp    = 0;
	m_pp.raid_leadership_exp     = 0;

	database.DeleteCharacterLeadershipAbilities(CharacterID());
}

void Client::SendClearPlayerAA()
{
	auto outapp = new EQApplicationPacket(OP_ClearAA, 0);
	FastQueuePacket(&outapp);
}

void Client::SendClearLeadershipAA()
{
	auto outapp = new EQApplicationPacket(OP_ClearLeadershipAbilities, 0);
	FastQueuePacket(&outapp);
}

int Client::GroupLeadershipAAHealthEnhancement()
{
	if (IsRaidGrouped()) {
		int bonus = 0;
		Raid *raid = GetRaid();
		if (!raid)
			return 0;
		uint32 group_id = raid->GetGroup(this);
		if (group_id < 12 && raid->GroupCount(group_id) >= 3) {
			switch (raid->GetLeadershipAA(groupAAHealthEnhancement, group_id)) {
			case 1:
				bonus = 30;
				break;
			case 2:
				bonus = 60;
				break;
			case 3:
				bonus = 100;
				break;
			}
		}
		if (raid->RaidCount() >= 18) {
			switch (raid->GetLeadershipAA(raidAAHealthEnhancement)) {
			case 1:
				bonus += 30;
				break;
			case 2:
				bonus += 60;
				break;
			case 3:
				bonus += 100;
				break;
			}
		}
		return bonus;
	}

	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAHealthEnhancement))
	{
		case 0:
			return 0;
		case 1:
			return 30;
		case 2:
			return 60;
		case 3:
			return 100;
	}

	return 0;
}

int Client::GroupLeadershipAAManaEnhancement()
{
	if (IsRaidGrouped()) {
		int bonus = 0;
		Raid *raid = GetRaid();
		if (!raid)
			return 0;
		uint32 group_id = raid->GetGroup(this);
		if (group_id < 12 && raid->GroupCount(group_id) >= 3) {
			switch (raid->GetLeadershipAA(groupAAManaEnhancement, group_id)) {
			case 1:
				bonus = 30;
				break;
			case 2:
				bonus = 60;
				break;
			case 3:
				bonus = 100;
				break;
			}
		}
		if (raid->RaidCount() >= 18) {
			switch (raid->GetLeadershipAA(raidAAManaEnhancement)) {
			case 1:
				bonus += 30;
				break;
			case 2:
				bonus += 60;
				break;
			case 3:
				bonus += 100;
				break;
			}
		}
		return bonus;
	}

	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAManaEnhancement))
	{
		case 0:
			return 0;
		case 1:
			return 30;
		case 2:
			return 60;
		case 3:
			return 100;
	}

	return 0;
}

int Client::GroupLeadershipAAHealthRegeneration()
{
	if (IsRaidGrouped()) {
		int bonus = 0;
		Raid *raid = GetRaid();
		if (!raid)
			return 0;
		uint32 group_id = raid->GetGroup(this);
		if (group_id < 12 && raid->GroupCount(group_id) >= 3) {
			switch (raid->GetLeadershipAA(groupAAHealthRegeneration, group_id)) {
			case 1:
				bonus = 4;
				break;
			case 2:
				bonus = 6;
				break;
			case 3:
				bonus = 8;
				break;
			}
		}
		if (raid->RaidCount() >= 18) {
			switch (raid->GetLeadershipAA(raidAAHealthRegeneration)) {
			case 1:
				bonus += 4;
				break;
			case 2:
				bonus += 6;
				break;
			case 3:
				bonus += 8;
				break;
			}
		}
		return bonus;
	}

	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAHealthRegeneration))
	{
		case 0:
			return 0;
		case 1:
			return 4;
		case 2:
			return 6;
		case 3:
			return 8;
	}

	return 0;
}

int Client::GroupLeadershipAAOffenseEnhancement()
{
	if (IsRaidGrouped()) {
		int bonus = 0;
		Raid *raid = GetRaid();
		if (!raid)
			return 0;
		uint32 group_id = raid->GetGroup(this);
		if (group_id < 12 && raid->GroupCount(group_id) >= 3) {
			switch (raid->GetLeadershipAA(groupAAOffenseEnhancement, group_id)) {
			case 1:
				bonus = 10;
				break;
			case 2:
				bonus = 19;
				break;
			case 3:
				bonus = 28;
				break;
			case 4:
				bonus = 34;
				break;
			case 5:
				bonus = 40;
				break;
			}
		}
		if (raid->RaidCount() >= 18) {
			switch (raid->GetLeadershipAA(raidAAOffenseEnhancement)) {
			case 1:
				bonus += 10;
				break;
			case 2:
				bonus += 19;
				break;
			case 3:
				bonus += 28;
				break;
			case 4:
				bonus += 34;
				break;
			case 5:
				bonus += 40;
				break;
			}
		}
		return bonus;
	}

	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAOffenseEnhancement))
	{
		case 0:
			return 0;
		case 1:
			return 10;
		case 2:
			return 19;
		case 3:
			return 28;
		case 4:
			return 34;
		case 5:
			return 40;
	}
	return 0;
}

void Client::InspectBuffs(Client* Inspector, int Rank)
{
	// At some point the removed the restriction of being a group member for this to work
	// not sure when, but the way it's coded now, it wouldn't work with mobs.
	if (!Inspector || Rank == 0)
		return;

	auto outapp = new EQApplicationPacket(OP_InspectBuffs, sizeof(InspectBuffs_Struct));
	InspectBuffs_Struct *ib = (InspectBuffs_Struct *)outapp->pBuffer;

	uint32 buff_count = GetMaxTotalSlots();
	uint32 packet_index = 0;
	for (uint32 i = 0; i < buff_count; i++) {
		if (!IsValidSpell(buffs[i].spellid))
			continue;
		ib->spell_id[packet_index] = buffs[i].spellid;
		if (Rank > 1)
			ib->tics_remaining[packet_index] = spells[buffs[i].spellid].buff_duration_formula == DF_Permanent ? 0xFFFFFFFF : buffs[i].ticsremaining;
		packet_index++;
	}

	Inspector->FastQueuePacket(&outapp);
}

void Client::RefundAA() {
	int refunded = 0;

	auto rank_value = aa_ranks.begin();
	while (rank_value != aa_ranks.end()) {
		auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(rank_value->first, rank_value->second.first);
		auto ability      = ability_rank.first;
		auto rank         = ability_rank.second;

		if (!ability) {
			++rank_value;
			continue;
		}

		if (ability->charges > 0 && rank_value->second.second < 1) {
			++rank_value;
			continue;
		}

		if (ability->grant_only) {
			++rank_value;
			continue;
		}

		refunded += rank->total_cost;
		rank_value        = aa_ranks.erase(rank_value);
	}

	if (refunded > 0) {
		m_pp.aapoints += refunded;
		SaveAA();
		Save();
	}

	SendAlternateAdvancementTable();
	SendAlternateAdvancementPoints();
	SendAlternateAdvancementStats();
}

SwarmPet::SwarmPet()
{
	target = 0;
	owner_id = 0;
	duration = nullptr;
}

SwarmPet::~SwarmPet()
{
	target = 0;
	owner_id = 0;
	safe_delete(duration);
}

Mob *SwarmPet::GetOwner()
{
	return entity_list.GetMobID(owner_id);
}

//New AA
void Client::SendAlternateAdvancementTable() {
	for(auto &aa : zone->aa_abilities) {
		uint32 charges = 0;
		auto ranks = GetAA(aa.second->first_rank_id, &charges);
		if(ranks) {
			if(aa.second->GetMaxLevel(this) == ranks) {
				SendAlternateAdvancementRank(aa.first, ranks);
			} else {
				SendAlternateAdvancementRank(aa.first, ranks);
				SendAlternateAdvancementRank(aa.first, ranks + 1);
			}
		} else {
			SendAlternateAdvancementRank(aa.first, 1);
		}
	}
}

void Client::SendAlternateAdvancementRank(int aa_id, int level) {
	if(!zone)
		return;

	auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(aa_id, level);
	auto ability = ability_rank.first;
	auto rank = ability_rank.second;

	if(!ability) {
		return;
	}

	if(!(ability->classes & (1 << GetClass()))) {
		return;
	}

	if(!CanUseAlternateAdvancementRank(rank)) {
		return;
	}

	int size = sizeof(AARankInfo_Struct) + (sizeof(AARankEffect_Struct) * rank->effects.size()) + (sizeof(AARankPrereq_Struct) * rank->prereqs.size());
	auto outapp = new EQApplicationPacket(OP_SendAATable, size);
	AARankInfo_Struct *aai = (AARankInfo_Struct*)outapp->pBuffer;

	aai->id = rank->id;
	aai->upper_hotkey_sid = rank->upper_hotkey_sid;
	aai->lower_hotkey_sid = rank->lower_hotkey_sid;
	aai->title_sid = rank->title_sid;
	aai->desc_sid = rank->desc_sid;
	aai->cost = rank->cost;
	aai->seq = aa_id;
	aai->type = ability->type;
	aai->spell = rank->spell;
	aai->spell_type = rank->spell_type;
	aai->spell_refresh = rank->recast_time;
	aai->classes = ability->classes;
	aai->level_req = rank->level_req;
	aai->current_level = level;
	aai->max_level = ability->GetMaxLevel(this);
	aai->prev_id = rank->prev_id;

	if((rank->next && !CanUseAlternateAdvancementRank(rank->next)) || ability->charges > 0) {
		aai->next_id = -1;
	} else {
		aai->next_id = rank->next_id;
	}
	aai->total_cost = rank->total_cost;
	aai->expansion = rank->expansion;
	aai->category = ability->category;
	aai->charges = ability->charges;
	aai->grant_only = ability->grant_only;
	aai->total_effects = rank->effects.size();
	aai->total_prereqs = rank->prereqs.size();

	outapp->SetWritePosition(sizeof(AARankInfo_Struct));
	for(auto &effect : rank->effects) {
		outapp->WriteSInt32(effect.effect_id);
		outapp->WriteSInt32(effect.base_value);
		outapp->WriteSInt32(effect.limit_value);
		outapp->WriteSInt32(effect.slot);
	}

	for(auto &prereq : rank->prereqs) {
		outapp->WriteSInt32(prereq.first);
		outapp->WriteSInt32(prereq.second);
	}

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendAlternateAdvancementStats() {
	auto outapp = new EQApplicationPacket(OP_AAExpUpdate, sizeof(AltAdvStats_Struct));
	AltAdvStats_Struct *aps = (AltAdvStats_Struct *)outapp->pBuffer;
	aps->experience = (uint32)(((float)330.0f * (float)m_pp.expAA) / (float)GetRequiredAAExperience());
	aps->unspent = m_pp.aapoints;
	aps->percentage = m_epp.perAA;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendAlternateAdvancementPoints() {
	auto outapp = new EQApplicationPacket(OP_RespondAA, sizeof(AATable_Struct));
	AATable_Struct* aa2 = (AATable_Struct *)outapp->pBuffer;

	int i = 0;
	for(auto &aa : zone->aa_abilities) {
		uint32 charges = 0;
		auto ranks = GetAA(aa.second->first_rank_id, &charges);
		if(ranks) {
			AA::Rank *rank = aa.second->GetRankByPointsSpent(ranks);
			if(rank) {
				aa2->aa_list[i].AA = rank->id;
				aa2->aa_list[i].value = rank->total_cost;
				aa2->aa_list[i].charges = charges;
				i++;
			}
		}
	}


	aa2->aa_spent = GetSpentAA();
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendAlternateAdvancementTimer(int ability, int begin, int end) {
	auto outapp = new EQApplicationPacket(OP_AAAction, sizeof(UseAA_Struct));
	UseAA_Struct* uaaout = (UseAA_Struct*)outapp->pBuffer;
	uaaout->ability = ability;
	uaaout->begin = begin;
	uaaout->end = end;
	QueuePacket(outapp);
	safe_delete(outapp);
}

//sends all AA timers.
void Client::SendAlternateAdvancementTimers() {
	//we dont use SendAATimer because theres no reason to allocate the EQApplicationPacket every time
	auto outapp = new EQApplicationPacket(OP_AAAction, sizeof(UseAA_Struct));
	UseAA_Struct* uaaout = (UseAA_Struct*)outapp->pBuffer;

	PTimerList::iterator c, e;
	c = p_timers.begin();
	e = p_timers.end();
	for(; c != e; ++c) {
		PersistentTimer *cur = c->second;
		if(cur->GetType() < pTimerAAStart || cur->GetType() > pTimerAAEnd)
			continue;	//not an AA timer
		//send timer
		uaaout->begin = cur->GetStartTime();
		uaaout->end = static_cast<uint32>(time(nullptr));
		uaaout->ability = cur->GetType() - pTimerAAStart; // uuaaout->ability is really a shared timer number
		QueuePacket(outapp);
	}

	safe_delete(outapp);
}

void Client::ResetAlternateAdvancementTimer(int ability) {
	AA::Rank *rank = zone->GetAlternateAdvancementRank(casting_spell_aa_id);
	if(rank) {
		SendAlternateAdvancementTimer(rank->spell_type, 0, time(0));
		p_timers.Clear(&database, rank->spell_type + pTimerAAStart);
	}
}

void Client::ResetAlternateAdvancementTimers() {
	auto outapp = new EQApplicationPacket(OP_AAAction, sizeof(UseAA_Struct));
	UseAA_Struct* uaaout = (UseAA_Struct*)outapp->pBuffer;

	PTimerList::iterator c, e;
	c = p_timers.begin();
	e = p_timers.end();
	std::vector<int> r_timers;
	for(; c != e; ++c) {
		PersistentTimer *cur = c->second;
		if(cur->GetType() < pTimerAAStart || cur->GetType() > pTimerAAEnd)
			continue;
		//send timer
		uaaout->begin = 0;
		uaaout->end = static_cast<uint32>(time(nullptr));
		uaaout->ability = cur->GetType() - pTimerAAStart;
		r_timers.push_back(cur->GetType());
		QueuePacket(outapp);
	}

	for(auto &i : r_timers) {
		p_timers.Clear(&database, i);
	}

	safe_delete(outapp);
}

void Client::ResetOnDeathAlternateAdvancement() {
	for (const auto &aa : aa_ranks) {
		auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(aa.first, aa.second.first);
		auto ability = ability_rank.first;
		auto rank = ability_rank.second;

		if (!ability)
			continue;

		if (!rank)
			continue;

		// since they're dying, we just need to clear the DB
		if (ability->reset_on_death)
			p_timers.Clear(&database, rank->spell_type + pTimerAAStart);
	}
}

void Client::PurchaseAlternateAdvancementRank(int rank_id) {
	AA::Rank *rank = zone->GetAlternateAdvancementRank(rank_id);
	if(!rank) {
		return;
	}

	if(!rank->base_ability) {
		return;
	}

	if(!CanPurchaseAlternateAdvancementRank(rank, true, true)) {
		return;
	}

	FinishAlternateAdvancementPurchase(rank, false, true);
}

bool Client::GrantAlternateAdvancementAbility(int aa_id, int points, bool ignore_cost) {
	bool ret = false;
	for(int i = 1; i <= points; ++i) {
		auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(aa_id, i);
		auto ability = ability_rank.first;
		auto rank = ability_rank.second;

		if(!rank) {
			continue;
		}

		if(!rank->base_ability) {
			continue;
		}

		if(!CanPurchaseAlternateAdvancementRank(rank, !ignore_cost, false)) {
			continue;
		}

		ret = true;
		FinishAlternateAdvancementPurchase(rank, ignore_cost, true);
	}

	return ret;
}

void Client::FinishAlternateAdvancementPurchase(AA::Rank *rank, bool ignore_cost, bool send_message_and_save) {
	auto rank_id = rank->base_ability->first_rank_id;

	if (rank->base_ability->charges) {
		uint32 charges = 0;
		GetAA(rank_id, &charges);

		if (charges) {
			return;
		}

		SetAA(rank_id, rank->current_value, rank->base_ability->charges);
	} else {
		SetAA(rank_id, rank->current_value, 0);

		//if not max then send next aa
		if (rank->next && send_message_and_save) {
			SendAlternateAdvancementRank(rank->base_ability->id, rank->next->current_value);
		}
	}

	auto cost = !ignore_cost ? rank->cost : 0;

	m_pp.aapoints -= static_cast<uint32>(cost);

	if (send_message_and_save) {
		SaveAA();
		SendAlternateAdvancementPoints();
		SendAlternateAdvancementStats();
	}

	if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::AA_PURCHASE)) {
		auto e = PlayerEvent::AAPurchasedEvent{
			.aa_id = rank->id,
			.aa_cost = cost,
			.aa_previous_id = rank->prev_id,
			.aa_next_id = rank->next_id
		};

		RecordPlayerEventLog(PlayerEvent::AA_PURCHASE, e);
	}

	if (rank->prev) {
		if (send_message_and_save) {
			MessageString(
				Chat::Yellow,
				AA_IMPROVE,
				std::to_string(rank->title_sid).c_str(),
				std::to_string(rank->prev->current_value).c_str(),
				std::to_string(cost).c_str(),
				cost == 1 ? std::to_string(AA_POINT).c_str() : std::to_string(AA_POINTS).c_str()
			);
		}
	} else {
		if (send_message_and_save) {
			MessageString(
				Chat::Yellow,
				AA_GAIN_ABILITY,
				std::to_string(rank->title_sid).c_str(),
				std::to_string(cost).c_str(),
				cost == 1 ? std::to_string(AA_POINT).c_str() : std::to_string(AA_POINTS).c_str()
			);
		}
	}

	if (parse->PlayerHasQuestSub(EVENT_AA_BUY)) {
		const auto& export_string = fmt::format(
			"{} {} {} {}",
			cost,
			rank->id,
			rank->prev_id,
			rank->next_id
		);

		parse->EventPlayer(EVENT_AA_BUY, this, export_string, 0);
	}

	CalcBonuses();

	if (cost) {
		if (title_manager.IsNewAATitleAvailable(m_pp.aapoints_spent, GetBaseClass())) {
			NotifyNewTitlesAvailable();
		}
	}
}

//need to rewrite this
void Client::IncrementAlternateAdvancementRank(int rank_id) {
	AA::Rank *rank = zone->GetAlternateAdvancementRank(rank_id);
	if(!rank) {
		return;
	}

	if(!rank->base_ability) {
		return;
	}

	int points = GetAA(rank_id);
	GrantAlternateAdvancementAbility(rank->base_ability->id, points + 1, true);
}

void Client::ActivateAlternateAdvancementAbility(int rank_id, int target_id) {
	AA::Rank *rank = zone->GetAlternateAdvancementRank(rank_id);

	if (!rank) {
		return;
	}

	AA::Ability *ability = rank->base_ability;
	if (!ability) {
		return;
	}

	if (!IsValidSpell(rank->spell)) {
		return;
	}

	if (!CanUseAlternateAdvancementRank(rank)) {
		return;
	}

	bool use_toggle_passive_hotkey = UseTogglePassiveHotkey(*rank);

	//make sure it is not a passive
	if (!rank->effects.empty() && !use_toggle_passive_hotkey) {
		return;
	}

	uint32 charges = 0;
	// We don't have the AA
	if (!GetAA(rank_id, &charges))
		return;
	//if expendable make sure we have charges
	if (ability->charges > 0 && charges < 1)
		return;

	//check cooldown
	if (!p_timers.Expired(&database, rank->spell_type + pTimerAAStart, false)) {
		uint32 aaremain = p_timers.GetRemainingTime(rank->spell_type + pTimerAAStart);
		uint32 aaremain_hr = aaremain / (60 * 60);
		uint32 aaremain_min = (aaremain / 60) % 60;
		uint32 aaremain_sec = aaremain % 60;

		if (aaremain_hr >= 1) {
			Message(Chat::Red, "You can use this ability again in %u hour(s) %u minute(s) %u seconds",
				aaremain_hr, aaremain_min, aaremain_sec);
		}
		else {
			Message(Chat::Red, "You can use this ability again in %u minute(s) %u seconds",
				aaremain_min, aaremain_sec);
		}

		return;
	}

	int timer_duration = rank->recast_time - GetAlternateAdvancementCooldownReduction(rank);
	if (timer_duration < 0) {
		timer_duration = 0;
	}

	if (!IsCastWhileInvisibleSpell(rank->spell))
		CommonBreakInvisible();

	if (spells[rank->spell].sneak && (!hidden || (hidden && (Timer::GetCurrentTime() - tmHidden) < 4000))) {
		MessageString(Chat::SpellFailure, SNEAK_RESTRICT);
		return;
	}
	//
	// Modern clients don't require pet targeted for AA casts that are ST_Pet
	if (spells[rank->spell].target_type == ST_Pet || spells[rank->spell].target_type == ST_SummonedPet)
		target_id = GetPetID();

	// extra handling for cast_not_standing spells
	if (!IsCastNotStandingSpell(rank->spell)) {
		if (GetAppearance() == eaSitting) // we need to stand!
			SetAppearance(eaStanding, false);

		if (GetAppearance() != eaStanding) {
			MessageString(Chat::SpellFailure, STAND_TO_CAST);
			return;
		}
	}

	if (use_toggle_passive_hotkey) {
		TogglePassiveAlternativeAdvancement(*rank, ability->id);
	}
	else {
		// Bards can cast instant cast AAs while they are casting or channeling item cast.
		if (GetClass() == Class::Bard && IsCasting() && spells[rank->spell].cast_time == 0) {
			if (!DoCastingChecksOnCaster(rank->spell, EQ::spells::CastingSlot::AltAbility)) {
				return;
			}

			if (!SpellFinished(rank->spell, entity_list.GetMob(target_id), EQ::spells::CastingSlot::AltAbility, spells[rank->spell].mana, -1, spells[rank->spell].resist_difficulty, false, -1,
				rank->spell_type + pTimerAAStart, timer_duration, false, rank->id)) {
				return;
			}
		}
		else {
			if (!CastSpell(rank->spell, target_id, EQ::spells::CastingSlot::AltAbility, -1, -1, 0, -1, rank->spell_type + pTimerAAStart, timer_duration, nullptr, rank->id)) {
				return;
			}
		}
	}
}

int Mob::GetAlternateAdvancementCooldownReduction(AA::Rank *rank_in) {
	if(!rank_in) {
		return 0;
	}

	AA::Ability *ability_in = rank_in->base_ability;
	if(!ability_in) {
		return 0;
	}

	int total_reduction = 0;
	for(auto &aa : aa_ranks) {
		auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(aa.first, aa.second.first);
		auto ability = ability_rank.first;
		auto rank = ability_rank.second;

		if(!ability) {
			continue;
		}

		for(auto &effect : rank->effects) {
			if(effect.effect_id == SpellEffect::HastenedAASkill && effect.limit_value == ability_in->id) {
				total_reduction += effect.base_value;
			}
		}
	}

	return total_reduction;
}

void Mob::ExpendAlternateAdvancementCharge(uint32 aa_id) {
	for (auto &iter : aa_ranks) {
		auto ability = zone->GetAlternateAdvancementAbility(iter.first);
		if (ability && aa_id == ability->id) {
			if (iter.second.second > 0) {
				iter.second.second -= 1;

				if (iter.second.second == 0) {
					if (IsClient()) {
						auto c = CastToClient();

						auto r = ability->GetRankByPointsSpent(iter.second.first);
						if (r) {
							c->GetEPP().expended_aa += r->cost;
						}

						c->RemoveExpendedAA(ability->first_rank_id);
					}

					aa_ranks.erase(iter.first);
				}

				if (IsClient()) {
					auto c = CastToClient();
					c->SaveAA();
					c->SendAlternateAdvancementPoints();
				}
			}

			return;
		}
	}
}

bool ZoneDatabase::LoadAlternateAdvancement(Client *c) {
	c->ClearAAs();

	const auto& l = CharacterAlternateAbilitiesRepository::GetWhere(
		database,
		fmt::format(
			"`id` = {}",
			c->CharacterID()
		)
	);

	uint32 slot_id = 0;

	for (const auto& e : l) {
		const uint16 aa_id    = e.aa_id;
		const uint16 aa_value = e.aa_value;
		const uint16 charges  = e.charges;

		auto rank = zone->GetAlternateAdvancementRank(aa_id);
		if (!rank) {
			continue;
		}

		auto ability = rank->base_ability;
		if (!ability) {
			continue;
		}

		rank = ability->GetRankByPointsSpent(aa_value);

		if (c->CanUseAlternateAdvancementRank(rank)) {
			c->GetPP().aa_array[slot_id].AA      = aa_id;
			c->GetPP().aa_array[slot_id].value   = aa_value;
			c->GetPP().aa_array[slot_id].charges = charges;

			c->SetAA(aa_id, aa_value, charges);

			slot_id++;
		}
	}

	return true;
}

AA::Ability *Zone::GetAlternateAdvancementAbility(int id) {
	auto iter = aa_abilities.find(id);
	if(iter != aa_abilities.end()) {
		return iter->second.get();
	}

	return nullptr;
}

AA::Ability *Zone::GetAlternateAdvancementAbilityByRank(int rank_id) {
	AA::Rank *rank = GetAlternateAdvancementRank(rank_id);

	if(!rank)
		return nullptr;

	return rank->base_ability;
}

AA::Rank *Zone::GetAlternateAdvancementRank(int rank_id) {
	auto iter = aa_ranks.find(rank_id);
	if(iter != aa_ranks.end()) {
		return iter->second.get();
	}

	return nullptr;
}

std::pair<AA::Ability*, AA::Rank*> Zone::GetAlternateAdvancementAbilityAndRank(int id, int points_spent) {
	AA::Ability *ability = GetAlternateAdvancementAbility(id);

	if(!ability) {
		return std::make_pair(nullptr, nullptr);
	}

	AA::Rank *rank = ability->GetRankByPointsSpent(points_spent);
	if(!rank) {
		return std::make_pair(nullptr, nullptr);
	}

	return std::make_pair(ability, rank);
}

uint32 Mob::GetAA(uint32 rank_id, uint32 *charges) const {
	if(zone) {
		AA::Ability *ability = zone->GetAlternateAdvancementAbilityByRank(rank_id);
		if(!ability)
			return 0;

		auto iter = aa_ranks.find(ability->id);
		if(iter != aa_ranks.end()) {
			if(charges) {
				*charges = iter->second.second;
			}
			return iter->second.first;
		}
	}
	return 0;
}

uint32 Mob::GetAAByAAID(uint32 aa_id, uint32 *charges) const {
	if(zone) {
		AA::Ability *ability = zone->GetAlternateAdvancementAbility(aa_id);

		if(!ability)
			return 0;

		auto iter = aa_ranks.find(ability->id);
		if(iter != aa_ranks.end()) {
			if(charges) {
				*charges = iter->second.second;
			}
			return iter->second.first;
		}
	}

	return 0;
}

bool Mob::SetAA(uint32 rank_id, uint32 new_value, uint32 charges)
{
	if (zone) {
		auto a = zone->GetAlternateAdvancementAbilityByRank(rank_id);

		if (!a) {
			return false;
		}

		if(new_value > a->GetMaxLevel(this)) {
			return false;
		}

		aa_ranks[a->id] = std::make_pair(new_value, charges);
	}

	return true;
}


bool Mob::CanUseAlternateAdvancementRank(AA::Rank *rank)
{
	if (!rank) {
		return false;
	}

	const auto a = rank->base_ability;

	if (!a) {
		return false;
	}

	if (!(a->classes & (1 << GetClass()))) {
		return false;
	}

	// Passive and Active Shroud AAs, skip for now
	if (
		a->category == AACategory::ShroudPassive ||
		a->category == AACategory::ShroudActive
	) {
		return false;
	}

	//the one titanium hack i will allow
	//just to make sure we dont crash the client with newer aas
	//we'll exclude any expendable ones
	if (IsClient() && CastToClient()->ClientVersionBit() & EQ::versions::maskTitaniumAndEarlier) {
		if (a->charges > 0) {
			return false;
		}
	}

	const int  expansion        = RuleI(Expansion, CurrentExpansion);
	const bool use_expansion_aa = RuleB(Expansion, UseCurrentExpansionAAOnly);
	if (use_expansion_aa && expansion >= 0) {
		if (rank->expansion > expansion) {
			return false;
		}
	}


	if (IsClient()) {
		if (rank->expansion && !(CastToClient()->GetPP().expansions & (1 << (rank->expansion - 1)))) {
			return false;
		}
	} else if (IsBot()) {
		if (rank->expansion && !(CastToBot()->GetExpansionBitmask() & (1 << (rank->expansion - 1)))) {
			return false;
		}
	} else {
		if (rank->expansion && !(RuleI(World, ExpansionSettings) & (1 << (rank->expansion - 1)))) {
			return false;
		}
	}

	auto race = GetPlayerRaceValue(GetBaseRace());

	race = race > RaceIndex::Drakkin ? Race::Human : race;

	if (!(a->races & (1 << (race - 1)))) {
		return false;
	}

	const auto deity = GetDeityBit();
	if (!(a->deities & deity)) {
		return false;
	}

	if (IsClient() && CastToClient()->Admin() < a->status) {
		return false;
	}

	if (GetBaseRace() == Race::Drakkin) {
		if (!(a->drakkin_heritage & (1 << GetDrakkinHeritage()))) {
			return false;
		}
	}

	return true;
}

bool Mob::CanPurchaseAlternateAdvancementRank(AA::Rank *rank, bool check_price, bool check_grant)
{
	auto a = rank->base_ability;

	if (!a) {
		return false;
	}

	if (!CanUseAlternateAdvancementRank(rank)) {
		return false;
	}

	if (IsClient() && CastToClient()->HasAlreadyPurchasedRank(rank)) {
		return false;
	}

	// You cannot purchase grant only AAs they can only be assigned
	if (check_grant && a->grant_only) {
		return false;
	}

	if (rank->level_req > GetLevel()) {
		return false;
	}

	uint32       current_charges = 0;
	const uint32 points          = GetAA(rank->id, &current_charges);

	//check that we are on previous rank already (if exists)
	//grant ignores the req to own the previous rank.
	if (check_grant && rank->prev) {
		if (points != rank->prev->current_value) {
			return false;
		}
	}

	//check that we aren't already on this rank or one ahead of us
	if (points >= rank->current_value) {
		return false;
	}

	//if expendable only let us purchase if we have no charges already
	//not quite sure on how this functions client side atm
	//I intend to look into it later to make sure the behavior is right
	if (a->charges > 0 && current_charges > 0) {
		return false;
	}

	//check prereqs
	for (auto &prereq: rank->prereqs) {
		AA::Ability *prereq_ability = zone->GetAlternateAdvancementAbility(prereq.first);

		if (prereq_ability) {
			auto ranks = GetAA(prereq_ability->first_rank_id);
			if (ranks < prereq.second) {
				return false;
			}
		}
	}

	//check price, if client
	if (check_price && IsClient()) {
		if (rank->cost > CastToClient()->GetAAPoints()) {
			return false;
		}
	}

	return true;
}

void Zone::LoadAlternateAdvancement() {
	if (!content_db.LoadAlternateAdvancementAbilities(aa_abilities, aa_ranks)) {
		aa_abilities.clear();
		aa_ranks.clear();
		LogInfo("Failed to load Alternate Advancement Data");
		return;
	}

	for(const auto &ability : aa_abilities) {
		ability.second->first = GetAlternateAdvancementRank(ability.second->first_rank_id);

		//process these ranks
		AA::Rank *current = ability.second->first;
		int i = 1;
		int prev_id = -1;
		while(current) {
			current->prev_id = prev_id;
			current->prev = GetAlternateAdvancementRank(current->prev_id);
			current->next = GetAlternateAdvancementRank(current->next_id);
			current->base_ability = ability.second.get();
			current->current_value = i;

			if(current->prev) {
				current->total_cost = current->cost + current->prev->total_cost;

				//check prereqs here
				for(auto &prev_prereq : current->prev->prereqs) {
					//if prev has an aa we dont have set
					//	then set it here too
					//if prev has an aa we have
					//	then set to whichever is highest

					auto iter = current->prereqs.find(prev_prereq.first);
					if(iter == current->prereqs.end()) {
						//not found
						current->prereqs[prev_prereq.first] = prev_prereq.second;
					} else {
						//they already have it too!
						auto points = std::max(iter->second, prev_prereq.second);
						current->prereqs[iter->first] = points;
					}
				}
			}
			else {
				current->prev_id = -1;
				current->total_cost = current->cost;
			}

			if(!current->next) {
				current->next_id = -1;
			}

			i++;
			prev_id = current->id;
			current = current->next;
		}
	}
}

bool ZoneDatabase::LoadAlternateAdvancementAbilities(
	std::unordered_map<int, std::unique_ptr<AA::Ability>> &abilities,
	std::unordered_map<int, std::unique_ptr<AA::Rank>> &ranks
)
{
	abilities.clear();

	const auto& aa_abilities = AaAbilityRepository::GetWhere(*this, "`enabled` = 1");

	if (aa_abilities.empty()) {
		LogError("Failed to load Alternate Advancement Abilities.");
		return false;
	}

	for (const auto& e : aa_abilities) {
		auto a = new AA::Ability;

		a->id                 = e.id;
		a->name               = e.name;
		a->category           = e.category;
		a->classes            = e.classes << 1; // EQ client has classes left shifted by one bit
		a->races              = e.races;
		a->deities            = e.deities;
		a->drakkin_heritage   = e.drakkin_heritage;
		a->status             = e.status;
		a->type               = e.type;
		a->charges            = e.charges;
		a->grant_only         = e.grant_only;
		a->reset_on_death     = e.reset_on_death;
		a->auto_grant_enabled = e.auto_grant_enabled;
		a->first_rank_id      = e.first_rank_id;
		a->first              = nullptr;

		abilities[a->id] = std::unique_ptr<AA::Ability>(a);
	}

	LogInfo(
		"Loaded [{}] Alternate Advancement Abilit{}.",
		Strings::Commify(abilities.size()),
		abilities.size() != 1 ? "ies" : "y"
	);

	ranks.clear();

	const auto& aa_ranks = AaRanksRepository::All(*this);

	if (aa_ranks.empty()) {
		LogError("Failed to load Alternate Advancement Ability Ranks.");
		return false;
	}

	for (const auto &e : aa_ranks) {
		auto r = new AA::Rank;

		r->id               = e.id;
		r->upper_hotkey_sid = e.upper_hotkey_sid;
		r->lower_hotkey_sid = e.lower_hotkey_sid;
		r->title_sid        = e.title_sid;
		r->desc_sid         = e.desc_sid;
		r->cost             = e.cost;
		r->level_req        = e.level_req;
		r->spell            = e.spell;
		r->spell_type       = e.spell_type;
		r->recast_time      = e.recast_time;
		r->next_id          = e.next_id;
		r->expansion        = e.expansion;
		r->base_ability     = nullptr;
		r->total_cost       = 0;
		r->prev_id          = -1;
		r->next             = nullptr;
		r->prev             = nullptr;

		ranks[r->id] = std::unique_ptr<AA::Rank>(r);
	}

	LogInfo(
		"Loaded [{}] Alternate Advancement Ability Rank{}.",
		Strings::Commify(ranks.size()),
		ranks.size() != 1 ? "s" : ""
	);

	const auto& aa_rank_effects = AaRankEffectsRepository::All(*this);

	if (aa_rank_effects.empty()) {
		LogError("Failed to load Alternate Advancement Ability Rank Effects.");
		return false;
	}

	for (const auto &e : aa_rank_effects) {
		AA::RankEffect f;

		f.slot        = e.slot;
		f.effect_id   = e.effect_id;
		f.base_value  = e.base1;
		f.limit_value = e.base2;

		if (f.slot < 1) {
			continue;
		}

		if (ranks.count(e.rank_id) > 0) {
			AA::Rank* r = ranks[e.rank_id].get();
			r->effects.push_back(f);
		}
	}

	LogInfo(
		"Loaded [{}] Alternate Advancement Ability Rank Effect{}.",
		Strings::Commify(aa_rank_effects.size()),
		aa_rank_effects.size() != 1 ? "s" : ""
	);

	const auto& aa_rank_prereqs = AaRankPrereqsRepository::All(*this);

	if (aa_rank_prereqs.empty()) {
		LogError("Failed to load Alternate Advancement Ability Rank Prerequisites.");
		return false;
	}

	for (const auto& e : aa_rank_prereqs) {
		if (e.aa_id <= 0 || e.points <= 0) {
			continue;
		}

		if (ranks.count(e.rank_id) > 0) {
			AA::Rank* rank = ranks[e.rank_id].get();
			rank->prereqs[e.aa_id] = e.points;
		}
	}

	LogInfo(
		"Loaded [{}] Alternate Advancement Ability Rank Prerequisite{}.",
		Strings::Commify(aa_rank_prereqs.size()),
		aa_rank_prereqs.size() != 1 ? "s" : ""
	);

	return true;
}

bool Mob::CheckAATimer(int timer)
{
	if (timer >= aaTimerMax)
		return false;
	if (aa_timers[timer].Enabled()) {
		if (aa_timers[timer].Check(false)) {
			aa_timers[timer].Disable();
			return false;
		} else {
			return true;
		}
	}
	return false;
}

void Client::TogglePassiveAlternativeAdvancement(const AA::Rank &rank, uint32 ability_id)
{
	/*
		Certain AA, like Weapon Stance line use a special toggle Hotkey to enable or disable the AA's passive abilities.
		This is occurs by doing the following. Each 'rank' of Weapon Stance is actually 2 actual ranks.
		First rank is always the Disabled version which cost X amount of AA. Second rank is the Enabled version which cost 0 AA.
		When you buy the first rank, you make a hotkey that on live say 'Weapon Stance Disabled', if you clik that it then BUYS the
		next rank of AA (cost 0) which switches the hotkey to 'Enabled Weapon Stance' and you are given the passive buff effects.
		If you click the Enabled hotkey, it causes you to lose an AA rank and once again be disabled. Thus, you are switching between
		two AA ranks. Thefore when creating an AA using this ability, you need generate both ranks. Follow the same pattern for additional ranks.

		IMPORTANT! The toggle system can be used to Enable or Disable ANY passive AA. You just need to follow the instructions on how to create it.
		Example: Enable or Disable a buff that gives a large hate modifier. Play may Enable when tanking and Disable when DPS ect.

		Note: On live the Enabled rank is shown having a Charge of 1, while Disabled rank has no charges. Our current code doesn't support that. Do not use charges.
		Note: Live uses a spell 'Disable Ability' ID 46164 to trigger a script to do the AA rank changes. At present time it is not coded to require that, any spell id works.
		Note: Discovered a bug on ROF2, where when you buy first rank of an AA with a hotkey, it will always display the title of the second rank in the database. Be aware. No easy fix.

		Dev Note(Kayen 8/1/21): The system as set up is very similar to live, with exception that live gives the Enabled rank 1 Charge. The code here emulates what happens when a
		charge would be expended.

		Instructions for how to make the AA - assuming a basic level of knowledge of how AA's work.
		- aa_abilities table : Create new ability with a hotkey, type 3, zero charges
		- aa_ranks table :  [Disabled rank] First rank, should have a cost > 0 (this is what you buy), Set hotkeys, MUST SET A SPELL CONTAINING EFFECT SpellEffect::Buy_AA_Rank(SPA 472), set a short recast timer.
							[Enabled rank] Second rank, should have a cost = 0, Set hotkeys, Set any valid spell ID you want (it has to exist but does nothing), set a short recast timer.
							*Recommend if doing custom, just make the hotkey titled 'Toggle <Ability Name>' and use for both.

		- aa_rank_effects table : [Disabled rank] No data needed in the aa_ranks_effect table
								  [Enabled rank]  Second rank set effect_id = 457 (weapon stance), slot 1,2,3, base1= spell triggers, base= weapon type (0=2H,1=SH,2=DW), for slot 1,2,3

			Example SQL			-Disabled
								DO NOT ADD any data to the aa_rank_effects for this rank_id

								-Enabled
								INSERT INTO aa_rank_effects (rank_id, slot, effect_id, base1, base2) VALUES (20003, 1, 476, 145,0);
								INSERT INTO aa_rank_effects (rank_id, slot, effect_id, base1, base2) VALUES (20003, 2, 476, 174,1);
								INSERT INTO aa_rank_effects (rank_id, slot, effect_id, base1, base2) VALUES (20003, 3, 476, 172,2);

		Warning: If you want to design an AA that only uses one weapon type to trigger, like will only apply buff if Shield. Do not include data for other types. Never have a base value=0
		in the Enabled rank.

	*/

	bool enable_next_rank = IsEffectInSpell(rank.spell, SpellEffect::Buy_AA_Rank);

	if (enable_next_rank) {

		//Enable
		TogglePurchaseAlternativeAdvancementRank(rank.next_id);
		Message(Chat::Spells, "You enable an ability."); //Message live gives you. Should come from spell.

		AA::Rank *rank_next = zone->GetAlternateAdvancementRank(rank.next_id);

		//Add checks for any special cases for toggle.
		if (rank_next && IsEffectinAlternateAdvancementRankEffects(*rank_next, SpellEffect::Weapon_Stance)) {
			weaponstance.aabonus_enabled = true;
			ApplyWeaponsStance();
		}
		return;
	}
	else {

		//Disable
		ResetAlternateAdvancementRank(ability_id);
		TogglePurchaseAlternativeAdvancementRank(rank.prev_id);
		Message(Chat::Spells, "You disable an ability."); //Message live gives you. Should come from spell.

		//Add checks for any special cases for toggle.
		if (IsEffectinAlternateAdvancementRankEffects(rank, SpellEffect::Weapon_Stance)) {
			weaponstance.aabonus_enabled = false;
			BuffFadeBySpellID(weaponstance.aabonus_buff_spell_id);
		}
		return;
	}
}

bool Client::UseTogglePassiveHotkey(const AA::Rank &rank) {

	/*
		Disabled rank needs a rank spell containing the SpellEffect::Buy_AA_Rank effect to return true.
		Enabled rank checks to see if the prior rank contains a rank spell with SpellEffect::Buy_AA_Rank, if so true.

		Note: On live the enabled rank is Expendable with Charge 1.

		We have already confirmed the rank spell is valid before this function is called.
	*/


	if (IsEffectInSpell(rank.spell, SpellEffect::Buy_AA_Rank)) {//Checked when is Disabled.
		return true;
	}
	else if (rank.prev_id != -1) {//Check when effect is Enabled.
		AA::Rank *rank_prev = zone->GetAlternateAdvancementRank(rank.prev_id);

		if (rank_prev && IsEffectInSpell(rank_prev->spell, SpellEffect::Buy_AA_Rank)) {
			return true;
		}
	}
	return false;
}

bool Client::IsEffectinAlternateAdvancementRankEffects(const AA::Rank &rank, int effect_id) {

	for (const auto &e : rank.effects) {

		if (e.effect_id == effect_id) {
			return true;
		}
	}
	return false;
}

void Client::ResetAlternateAdvancementRank(uint32 aa_id) {

	/*
		Resets your AA to baseline
	*/

	for(auto &iter : aa_ranks) {

		AA::Ability *ability = zone->GetAlternateAdvancementAbility(iter.first);

		if(ability && aa_id == ability->id) {
			RemoveExpendedAA(ability->first_rank_id);
			aa_ranks.erase(iter.first);
			SaveAA();
			SendAlternateAdvancementPoints();
			return;
		}
	}
}

void Client::TogglePurchaseAlternativeAdvancementRank(int rank_id){

	/*
		Stripped down version of purchasing AA. Will give no messages.
		Used with toggle hotkey functions.
	*/

	AA::Rank *rank = zone->GetAlternateAdvancementRank(rank_id);
	if (!rank) {
		return;
	}

	if (!rank->base_ability) {
		return;
	}

	if (!CanPurchaseAlternateAdvancementRank(rank, false, false)) {
		return;
	}

	rank_id = rank->base_ability->first_rank_id;
	SetAA(rank_id, rank->current_value, 0);

	if (rank->next) {
		SendAlternateAdvancementRank(rank->base_ability->id, rank->next->current_value);
	}

	SaveAA();
	SendAlternateAdvancementPoints();
	SendAlternateAdvancementStats();
	CalcBonuses();
}

void Client::AutoGrantAAPoints() {
	int auto_grant_expansion = RuleI(Expansion, AutoGrantAAExpansion);

	if (auto_grant_expansion == -1) {
		return;
	}

	//iterate through every AA
	for (auto& iter : zone->aa_abilities) {
		auto ability = iter.second.get();

		if (ability->grant_only) {
			continue;
		}

		if (ability->charges > 0) {
			continue;
		}

		if (!ability->auto_grant_enabled) {
			continue;
		}

		auto level = GetLevel();
		auto p = 1;
		auto rank = ability->first;
		while (rank != nullptr) {
			if (CanUseAlternateAdvancementRank(rank)) {
				if (rank->expansion <= auto_grant_expansion && rank->level_req <= level && !HasAlreadyPurchasedRank(rank)) {
					FinishAlternateAdvancementPurchase(rank, true, false);

					if (rank->prev) {
						MessageString(
							Chat::Yellow,
							AA_IMPROVE,
							std::to_string(rank->title_sid).c_str(),
							std::to_string(rank->prev->current_value).c_str(),
							"0",
							std::to_string(AA_POINTS).c_str()
						);
					}
					else {
						MessageString(
							Chat::Yellow,
							AA_GAIN_ABILITY,
							std::to_string(rank->title_sid).c_str(),
							"0",
							std::to_string(AA_POINTS).c_str()
						);
					}
				}
			}
			else {
				break;
			}

			p++;
			rank = rank->next;
		}
	}

	SendClearLeadershipAA();
	SendClearPlayerAA();
	SendAlternateAdvancementTable();
	SendAlternateAdvancementPoints();
	SendAlternateAdvancementStats();
}

void Client::GrantAllAAPoints(uint8 unlock_level, bool skip_grant_only)
{
	//iterate through every AA
	for (auto& aa : zone->aa_abilities) {
		AA::Ability* ability = aa.second.get();

		if (ability->charges > 0) {
			continue;
		}

		if (ability->grant_only && skip_grant_only) {
			continue;
		}

		const uint8 level = unlock_level ? unlock_level : GetLevel();

		AA::Rank* rank = ability->first;
		while (rank) {
			if (!CanUseAlternateAdvancementRank(rank)) {
				break;
			}

			if (rank->level_req <= level && !HasAlreadyPurchasedRank(rank)) {
				FinishAlternateAdvancementPurchase(rank, true, false);
			}

			rank = rank->next;
		}
	}

	SaveAA();
	SendClearLeadershipAA();
	SendClearPlayerAA();
	SendAlternateAdvancementTable();
	SendAlternateAdvancementPoints();
	SendAlternateAdvancementStats();
}

bool Client::HasAlreadyPurchasedRank(AA::Rank* rank) {
	const auto& aa = aa_ranks.find(rank->base_ability->id);
	if (aa == aa_ranks.end()) {
		return false;
	}

	const auto& ability_rank = zone->GetAlternateAdvancementAbilityAndRank(aa->first, aa->second.first);

	AA::Ability* ability = ability_rank.first;
	AA::Rank*    current = ability_rank.second;

	while (current) {
		if (current == rank) {
			return true;
		}

		current = current->prev;
	}

	return false;
}

void Client::ListPurchasedAAs(Client *to, std::string search_criteria)
{
	if (!to) {
		return;
	}

	std::map<std::string, uint8> client_aa_ranks;

	for (auto &aa : zone->aa_abilities) {
		AA::Ability *ability = aa.second.get();

		AA::Rank *rank = ability->first;
		while (rank) {
			if (!CanUseAlternateAdvancementRank(rank)) {
				break;
			}

			if (HasAlreadyPurchasedRank(rank)) {
				const std::string aa_name = zone->GetAAName(rank->id);
				if (
					search_criteria.empty() ||
					Strings::Contains(
						Strings::ToLower(aa_name),
						Strings::ToLower(search_criteria)
					)
				) {
					if (client_aa_ranks.find(aa_name) == client_aa_ranks.end()) {
						client_aa_ranks[aa_name] = 1;
					} else {
						client_aa_ranks[aa_name]++;
					}
				}
			}

			rank = rank->next;
		}
	}

	if (client_aa_ranks.empty()) {
		to->Message(
			Chat::White,
			fmt::format(
				"{} {} no purchased AAs{}.",
				to->GetTargetDescription(this, TargetDescriptionType::UCYou),
				this == to ? "have" : "has",
				(
					!search_criteria.empty() ?
					fmt::format(
						" matching '{}'",
						search_criteria
					) :
					""
				)
			).c_str()
		);
		return;
	}

	int aa_number = 1;

	for (const auto &aa : client_aa_ranks) {
		to->Message(
			Chat::White,
			fmt::format(
				"{}. {} (Rank {})",
				aa_number,
				aa.first,
				aa.second
			).c_str()
		);

		aa_number++;
	}

	to->Message(
		Chat::White,
		fmt::format(
			"{} {} {} purchased AA{}{}.",
			to->GetTargetDescription(this, TargetDescriptionType::UCYou),
			this == to ? "have" : "has",
			client_aa_ranks.size(),
			client_aa_ranks.size() > 1 ? "s" : "",
			(
				!search_criteria.empty() ?
				fmt::format(
					" matching '{}'",
					search_criteria
				) :
				""
			)
		).c_str()
	);
}
