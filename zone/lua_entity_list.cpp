#ifdef LUA_EQEMU

#include "lua_entity_list.h"

#include "zone/lua_bot.h"
#include "zone/lua_client.h"
#include "zone/lua_corpse.h"
#include "zone/lua_door.h"
#include "zone/lua_group.h"
#include "zone/lua_merc.h"
#include "zone/lua_mob.h"
#include "zone/lua_npc.h"
#include "zone/lua_object.h"
#include "zone/lua_raid.h"
#include "zone/lua_spawn.h"
#include "zone/masterentity.h"

#include "luabind/luabind.hpp"
#include "luabind/iterator_policy.hpp"

struct Lua_Mob_List {
	std::vector<Lua_Mob> entries;
};

struct Lua_NPC_List {
	std::vector<Lua_NPC> entries;
};

struct Lua_Client_List {
	std::vector<Lua_Client> entries;
};

struct Lua_Bot_List {
	std::vector<Lua_Bot> entries;
};

struct Lua_Corpse_List {
	std::vector<Lua_Corpse> entries;
};

struct Lua_Object_List {
	std::vector<Lua_Object> entries;
};

struct Lua_Doors_List {
	std::vector<Lua_Door> entries;
};

struct Lua_Spawn_List {
	std::vector<Lua_Spawn> entries;
};

Lua_Mob Lua_EntityList::GetMobID(int id) {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetMobID(id));
}

Lua_Mob Lua_EntityList::GetMob(const char *name) {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetMob(name));
}

Lua_Mob Lua_EntityList::GetMob(int id) {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetMob(id));
}

Lua_Mob Lua_EntityList::GetMobByNpcTypeID(int npc_type) {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetMobByNpcTypeID(npc_type));
}

bool Lua_EntityList::IsMobSpawnedByNpcTypeID(int npc_type) {
	Lua_Safe_Call_Bool();
	return self->IsMobSpawnedByNpcTypeID(npc_type);
}

Lua_NPC Lua_EntityList::GetNPCByID(int id) {
	Lua_Safe_Call_Class(Lua_NPC);
	return Lua_NPC(self->GetNPCByID(id));
}

Lua_NPC Lua_EntityList::GetNPCByNPCTypeID(int npc_type) {
	Lua_Safe_Call_Class(Lua_NPC);
	return Lua_NPC(self->GetNPCByNPCTypeID(npc_type));
}

Lua_NPC Lua_EntityList::GetNPCBySpawnID(uint32 spawn_id) {
	Lua_Safe_Call_Class(Lua_NPC);
	return Lua_NPC(self->GetNPCBySpawnID(spawn_id));
}

Lua_Client Lua_EntityList::GetClientByName(const char *name) {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetClientByName(name));
}

Lua_Client Lua_EntityList::GetClientByAccID(uint32 acct_id) {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetClientByAccID(acct_id));
}

Lua_Client Lua_EntityList::GetClientByID(int id) {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetClientByID(id));
}

Lua_Client Lua_EntityList::GetClientByCharID(uint32 char_id) {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetClientByCharID(char_id));
}

Lua_Client Lua_EntityList::GetClientByWID(uint32 wid) {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetClientByWID(wid));
}

Lua_Object Lua_EntityList::GetObjectByID(int id) {
	Lua_Safe_Call_Class(Lua_Object);
	return Lua_Object(self->GetObjectByID(id));
}

Lua_Object Lua_EntityList::GetObjectByDBID(uint32 db_id) {
	Lua_Safe_Call_Class(Lua_Object);
	return Lua_Object(self->GetObjectByDBID(db_id));
}

Lua_Door Lua_EntityList::GetDoorsByID(int id) {
	Lua_Safe_Call_Class(Lua_Door);
	return Lua_Door(self->GetDoorsByID(id));
}

Lua_Door Lua_EntityList::GetDoorsByDBID(uint32 db_id) {
	Lua_Safe_Call_Class(Lua_Door);
	return Lua_Door(self->GetDoorsByDBID(db_id));
}

Lua_Door Lua_EntityList::GetDoorsByDoorID(uint32 door_id) {
	Lua_Safe_Call_Class(Lua_Door);
	return Lua_Door(self->GetDoorsByDoorID(door_id));
}

Lua_Door Lua_EntityList::FindDoor(uint32 id) {
	Lua_Safe_Call_Class(Lua_Door);
	return Lua_Door(self->FindDoor(id));
}

Lua_Group Lua_EntityList::GetGroupByMob(Lua_Mob mob) {
	Lua_Safe_Call_Class(Lua_Group);
	return Lua_Group(self->GetGroupByMob(mob));
}

Lua_Group Lua_EntityList::GetGroupByClient(Lua_Client client) {
	Lua_Safe_Call_Class(Lua_Group);
	return Lua_Group(self->GetGroupByClient(client));
}

Lua_Group Lua_EntityList::GetGroupByID(int id) {
	Lua_Safe_Call_Class(Lua_Group);
	return Lua_Group(self->GetGroupByID(id));
}

Lua_Group Lua_EntityList::GetGroupByLeaderName(const char *name) {
	Lua_Safe_Call_Class(Lua_Group);
	return Lua_Group(self->GetGroupByLeaderName(name));
}

Lua_Raid Lua_EntityList::GetRaidByID(int id) {
	Lua_Safe_Call_Class(Lua_Raid);
	return Lua_Raid(self->GetRaidByID(id));
}

Lua_Raid Lua_EntityList::GetRaidByClient(Lua_Client client) {
	Lua_Safe_Call_Class(Lua_Raid);
	return Lua_Raid(self->GetRaidByClient(client));
}

Lua_Corpse Lua_EntityList::GetCorpseByOwner(Lua_Client client) {
	Lua_Safe_Call_Class(Lua_Corpse);
	return Lua_Corpse(self->GetCorpseByOwner(client));
}

Lua_Corpse Lua_EntityList::GetCorpseByID(int id) {
	Lua_Safe_Call_Class(Lua_Corpse);
	return Lua_Corpse(self->GetCorpseByID(id));
}

Lua_Corpse Lua_EntityList::GetCorpseByName(const char *name) {
	Lua_Safe_Call_Class(Lua_Corpse);
	return Lua_Corpse(self->GetCorpseByName(name));
}

Lua_Spawn Lua_EntityList::GetSpawnByID(uint32 id) {
	Lua_Safe_Call_Class(Lua_Spawn);
	return self->GetSpawnByID(id);
}

void Lua_EntityList::ClearClientPetitionQueue() {
	Lua_Safe_Call_Void();
	self->ClearClientPetitionQueue();
}

bool Lua_EntityList::CanAddHateForMob(Lua_Mob p) {
	Lua_Safe_Call_Bool();
	return self->CanAddHateForMob(p);
}

void Lua_EntityList::Message(uint32 guild_dbid, uint32 type, const char *message) {
	Lua_Safe_Call_Void();
	self->Message(guild_dbid, type, message);
}

void Lua_EntityList::MessageStatus(uint32 guild_dbid, int min_status, uint32 type, const char *message) {
	Lua_Safe_Call_Void();
	self->MessageStatus(
		guild_dbid,
		min_status,
		type,
		message
	);
}

void Lua_EntityList::MessageClose(Lua_Mob sender, bool skip_sender, float dist, uint32 type, const char *message) {
	Lua_Safe_Call_Void();

	if (RuleB(Chat, AutoInjectSaylinksToClientMessage))
	{
		std::string new_message = EQ::SayLinkEngine::InjectSaylinksIfNotExist(message);
		self->MessageClose(sender, skip_sender, dist, type, new_message.c_str());
	}
	else
	{
		self->MessageClose(sender, skip_sender, dist, type, message);
	}
}

void Lua_EntityList::FilteredMessageClose(Lua_Mob sender, bool skip_sender, float dist, uint32 type, int filter, const char *message)
{
	Lua_Safe_Call_Void();
	self->FilteredMessageClose(sender, skip_sender, dist, type, (eqFilterType)filter, message);
}

void Lua_EntityList::RemoveFromTargets(Lua_Mob mob) {
	Lua_Safe_Call_Void();
	self->RemoveFromTargets(mob);
}

void Lua_EntityList::RemoveFromTargets(Lua_Mob mob, bool RemoveFromXTargets) {
	Lua_Safe_Call_Void();
	self->RemoveFromTargets(mob, RemoveFromXTargets);
}

void Lua_EntityList::ReplaceWithTarget(Lua_Mob target, Lua_Mob new_target) {
	Lua_Safe_Call_Void();
	self->ReplaceWithTarget(target, new_target);
}

void Lua_EntityList::OpenDoorsNear(Lua_Mob opener) {
	Lua_Safe_Call_Void();
	self->OpenDoorsNear(opener);
}

std::string Lua_EntityList::MakeNameUnique(const char *name) {
	Lua_Safe_Call_String();

	char t_name[64];
	strncpy(t_name, name, 64);
	return self->MakeNameUnique(t_name);
}

std::string Lua_EntityList::RemoveNumbers(const char *name) {
	Lua_Safe_Call_String();

	char t_name[64];
	strncpy(t_name, name, 64);
	return self->RemoveNumbers(t_name);
}

void Lua_EntityList::SignalMobsByNPCID(uint32 npc_id, int signal_id) {
	Lua_Safe_Call_Void();
	self->SignalMobsByNPCID(npc_id, signal_id);
}

uint32 Lua_EntityList::DeleteNPCCorpses() {
	Lua_Safe_Call_Int();
	return self->DeleteNPCCorpses();
}

uint32 Lua_EntityList::DeletePlayerCorpses() {
	Lua_Safe_Call_Int();
	return self->DeletePlayerCorpses();
}

void Lua_EntityList::HalveAggro(Lua_Mob who) {
	Lua_Safe_Call_Void();
	self->HalveAggro(who);
}

void Lua_EntityList::DoubleAggro(Lua_Mob who) {
	Lua_Safe_Call_Void();
	self->DoubleAggro(who);
}

void Lua_EntityList::ClearFeignAggro(Lua_Mob who) {
	Lua_Safe_Call_Void();
	self->ClearFeignAggro(who);
}

bool Lua_EntityList::Fighting(Lua_Mob who) {
	Lua_Safe_Call_Bool();
	return self->Fighting(who);
}

void Lua_EntityList::RemoveFromHateLists(Lua_Mob who) {
	Lua_Safe_Call_Void();
	self->RemoveFromHateLists(who);
}

void Lua_EntityList::RemoveFromHateLists(Lua_Mob who, bool set_to_one) {
	Lua_Safe_Call_Void();
	self->RemoveFromHateLists(who, set_to_one);
}

void Lua_EntityList::MessageGroup(Lua_Mob who, bool skip_close, uint32 type, const char *message) {
	Lua_Safe_Call_Void();
	self->MessageGroup(who, skip_close, type, message);
}

Lua_Client Lua_EntityList::GetRandomClient() {
	Lua_Safe_Call_Class(Lua_Client);
	return self->GetRandomClient();
}

Lua_Client Lua_EntityList::GetRandomClient(float x, float y, float z, float distance) {
	Lua_Safe_Call_Class(Lua_Client);
	return self->GetRandomClient(glm::vec3(x, y, z), distance);
}

Lua_Client Lua_EntityList::GetRandomClient(float x, float y, float z, float distance, Lua_Client exclude_client) {
	Lua_Safe_Call_Class(Lua_Client);
	return self->GetRandomClient(glm::vec3(x, y, z), distance, exclude_client);
}

Lua_Mob_List Lua_EntityList::GetMobList() {
	Lua_Safe_Call_Class(Lua_Mob_List);
	Lua_Mob_List ret;
	auto &t_list = self->GetMobList();

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.emplace_back(Lua_Mob(iter->second));
		++iter;
	}

	return ret;
}

Lua_Client_List Lua_EntityList::GetClientList() {
	Lua_Safe_Call_Class(Lua_Client_List);
	Lua_Client_List ret;
	auto &t_list = self->GetClientList();

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.emplace_back(Lua_Client(iter->second));
		++iter;
	}

	return ret;
}

Lua_Bot Lua_EntityList::GetBotByID(uint32 bot_id) {
	Lua_Safe_Call_Class(Lua_Bot);
	return Lua_Bot(self->GetBotByBotID(bot_id));
}

Lua_Bot Lua_EntityList::GetBotByName(std::string bot_name) {
	Lua_Safe_Call_Class(Lua_Bot);
	return Lua_Bot(self->GetBotByBotName(bot_name));
}

Lua_Bot_List Lua_EntityList::GetBotList() {
	Lua_Safe_Call_Class(Lua_Bot_List);
	Lua_Bot_List ret;

	auto &current_bot_list = self->GetBotList();
	auto it = current_bot_list.begin();

	while (it != current_bot_list.end()) {
		ret.entries.emplace_back(it->second);
		++it;
	}

	return ret;
}

Lua_Client Lua_EntityList::GetBotOwnerByBotEntityID(uint32 entity_id) {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetBotOwnerByBotEntityID(entity_id));
}

Lua_Client Lua_EntityList::GetBotOwnerByBotID(uint32 bot_id) {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetBotOwnerByBotID(bot_id));
}

Lua_Bot_List Lua_EntityList::GetBotListByCharacterID(uint32 character_id) {
	Lua_Safe_Call_Class(Lua_Bot_List);
	Lua_Bot_List ret;
	auto bot_list = self->GetBotListByCharacterID(character_id);

	if (bot_list.size()) {
		for (auto bot : bot_list) {
			ret.entries.emplace_back(Lua_Bot(bot));
		}
	}

	return ret;
}

Lua_Bot_List Lua_EntityList::GetBotListByCharacterID(uint32 character_id, uint8 class_id) {
	Lua_Safe_Call_Class(Lua_Bot_List);
	Lua_Bot_List ret;
	auto bot_list = self->GetBotListByCharacterID(character_id, class_id);

	if (bot_list.size()) {
		for (auto bot : bot_list) {
			ret.entries.emplace_back(Lua_Bot(bot));
		}
	}

	return ret;
}

Lua_Bot_List Lua_EntityList::GetBotListByClientName(std::string client_name) {
	Lua_Safe_Call_Class(Lua_Bot_List);
	Lua_Bot_List ret;
	auto bot_list = self->GetBotListByClientName(client_name);

	if (bot_list.size()) {
		for (auto bot : bot_list) {
			ret.entries.emplace_back(Lua_Bot(bot));
		}
	}

	return ret;
}

Lua_Bot_List Lua_EntityList::GetBotListByClientName(std::string client_name, uint8 class_id) {
	Lua_Safe_Call_Class(Lua_Bot_List);
	Lua_Bot_List ret;
	auto bot_list = self->GetBotListByClientName(client_name, class_id);

	if (bot_list.size()) {
		for (auto bot : bot_list) {
			ret.entries.emplace_back(Lua_Bot(bot));
		}
	}

	return ret;
}

void Lua_EntityList::SignalAllBotsByOwnerCharacterID(uint32 character_id, int signal_id) {
	Lua_Safe_Call_Void();
	self->SignalAllBotsByOwnerCharacterID(character_id, signal_id);
}

void Lua_EntityList::SignalAllBotsByOwnerName(std::string owner_name, int signal_id) {
	Lua_Safe_Call_Void();
	self->SignalAllBotsByOwnerName(owner_name, signal_id);
}

void Lua_EntityList::SignalBotByBotID(uint32 bot_id, int signal_id) {
	Lua_Safe_Call_Void();
	self->SignalBotByBotID(bot_id, signal_id);
}

void Lua_EntityList::SignalBotByBotName(std::string bot_name, int signal_id) {
	Lua_Safe_Call_Void();
	self->SignalBotByBotName(bot_name, signal_id);
}

Lua_Client_List Lua_EntityList::GetShuffledClientList() {
	Lua_Safe_Call_Class(Lua_Client_List);
	Lua_Client_List ret;
	auto &t_list = self->GetClientList();

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.emplace_back(Lua_Client(iter->second));
		++iter;
	}

	zone->random.Shuffle(ret.entries.begin(), ret.entries.end());

	return ret;
}

Lua_NPC_List Lua_EntityList::GetNPCList() {
	Lua_Safe_Call_Class(Lua_NPC_List);
	Lua_NPC_List ret;
	auto &t_list = self->GetNPCList();

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.emplace_back(Lua_NPC(iter->second));
		++iter;
	}

	return ret;
}

Lua_Corpse_List Lua_EntityList::GetCorpseList() {
	Lua_Safe_Call_Class(Lua_Corpse_List);
	Lua_Corpse_List ret;
	auto &t_list = self->GetCorpseList();

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.emplace_back(Lua_Corpse(iter->second));
		++iter;
	}

	return ret;
}

Lua_Object_List Lua_EntityList::GetObjectList() {
	Lua_Safe_Call_Class(Lua_Object_List);
	Lua_Object_List ret;
	auto &t_list = self->GetObjectList();

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.emplace_back(Lua_Object(iter->second));
		++iter;
	}

	return ret;
}

Lua_Doors_List Lua_EntityList::GetDoorsList() {
	Lua_Safe_Call_Class(Lua_Doors_List);
	Lua_Doors_List ret;
	auto &t_list = self->GetDoorsList();

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.emplace_back(Lua_Door(iter->second));
		++iter;
	}

	return ret;
}

Lua_Spawn_List Lua_EntityList::GetSpawnList() {
	Lua_Safe_Call_Class(Lua_Spawn_List);
	Lua_Spawn_List ret;
	std::list<Spawn2*> t_list;
	self->GetSpawnList(t_list);

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.emplace_back(Lua_Spawn(*iter));
		++iter;
	}

	return ret;
}

void Lua_EntityList::SignalAllClients(int signal_id) {
	Lua_Safe_Call_Void();
	self->SignalAllClients(signal_id);
}

void Lua_EntityList::ChannelMessage(Lua_Mob from, int channel_num, uint8 language_id, const char *message) {
	Lua_Safe_Call_Void();
	self->ChannelMessage(from, channel_num, language_id, message);
}

Lua_Mob Lua_EntityList::GetRandomMob() {
	Lua_Safe_Call_Class(Lua_Mob);
	return self->GetRandomMob();
}

Lua_Mob Lua_EntityList::GetRandomMob(float x, float y, float z, float distance) {
	Lua_Safe_Call_Class(Lua_Mob);
	return self->GetRandomMob(glm::vec3(x, y, z), distance);
}

Lua_Mob Lua_EntityList::GetRandomMob(float x, float y, float z, float distance, Lua_Mob exclude_mob) {
	Lua_Safe_Call_Class(Lua_Mob);
	return self->GetRandomMob(glm::vec3(x, y, z), distance, exclude_mob);
}

Lua_NPC Lua_EntityList::GetRandomNPC() {
	Lua_Safe_Call_Class(Lua_NPC);
	return self->GetRandomNPC();
}

Lua_NPC Lua_EntityList::GetRandomNPC(float x, float y, float z, float distance) {
	Lua_Safe_Call_Class(Lua_NPC);
	return self->GetRandomNPC(glm::vec3(x, y, z), distance);
}

Lua_NPC Lua_EntityList::GetRandomNPC(float x, float y, float z, float distance, Lua_NPC exclude_npc) {
	Lua_Safe_Call_Class(Lua_NPC);
	return self->GetRandomNPC(glm::vec3(x, y, z), distance, exclude_npc);
}

void Lua_EntityList::Marquee(uint32 type, std::string message) {
	Lua_Safe_Call_Void();
	self->Marquee(type, message);
}

void Lua_EntityList::Marquee(uint32 type, std::string message, uint32 duration) {
	Lua_Safe_Call_Void();
	self->Marquee(type, message, duration);
}

void Lua_EntityList::Marquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string message) {
	Lua_Safe_Call_Void();
	self->Marquee(type, priority, fade_in, fade_out, duration, message);
}

Lua_Bot Lua_EntityList::GetRandomBot() {
	Lua_Safe_Call_Class(Lua_Bot);
	return self->GetRandomBot();
}

Lua_Bot Lua_EntityList::GetRandomBot(float x, float y, float z, float distance) {
	Lua_Safe_Call_Class(Lua_Bot);
	return self->GetRandomBot(glm::vec3(x, y, z), distance);
}

Lua_Bot Lua_EntityList::GetRandomBot(float x, float y, float z, float distance, Lua_Bot exclude_bot) {
	Lua_Safe_Call_Class(Lua_Bot);
	return self->GetRandomBot(glm::vec3(x, y, z), distance, exclude_bot);
}

Lua_Mob_List Lua_EntityList::GetCloseMobList(Lua_Mob mob) {
	Lua_Safe_Call_Class(Lua_Mob_List);

	Lua_Mob_List ret;

	const auto& l = self->GetCloseMobList(mob);
	ret.entries.reserve(l.size());
	for (const auto& e : l) {
		ret.entries.emplace_back(Lua_Mob(e.second));
	}

	return ret;
}

Lua_Mob_List Lua_EntityList::GetCloseMobList(Lua_Mob mob, float distance) {
	Lua_Safe_Call_Class(Lua_Mob_List);
	Lua_Mob_List ret;

	for (const auto& e : self->GetCloseMobList(mob)) {
		if (mob.CalculateDistance(e.second) <= distance) {
			ret.entries.emplace_back(Lua_Mob(e.second));
		}
	}

	return ret;
}

Lua_Mob_List Lua_EntityList::GetCloseMobList(Lua_Mob mob, float distance, bool ignore_self) {
	Lua_Safe_Call_Class(Lua_Mob_List);

	Lua_Mob_List ret;

	for (const auto& e : self->GetCloseMobList(mob)) {
		if (ignore_self && e.second == mob) {
			continue;
		}

		if (mob.CalculateDistance(e.second) <= distance) {
			ret.entries.emplace_back(Lua_Mob(e.second));
		}
	}

	return ret;
}

void Lua_EntityList::AreaAttack(Lua_Mob attacker, float distance)
{
	Lua_Safe_Call_Void();
	self->AEAttack(attacker, distance);
}

void Lua_EntityList::AreaAttack(Lua_Mob attacker, float distance, int16 slot_id)
{
	Lua_Safe_Call_Void();
	self->AEAttack(attacker, distance, slot_id);
}

void Lua_EntityList::AreaAttack(Lua_Mob attacker, float distance, int16 slot_id, int count)
{
	Lua_Safe_Call_Void();
	self->AEAttack(attacker, distance, slot_id, count);
}

void Lua_EntityList::AreaAttack(Lua_Mob attacker, float distance, int16 slot_id, int count, bool is_from_spell)
{
	Lua_Safe_Call_Void();
	self->AEAttack(attacker, distance, slot_id, count, is_from_spell);
}

void Lua_EntityList::AreaAttack(Lua_Mob attacker, float distance, int16 slot_id, int count, bool is_from_spell, int attack_rounds)
{
	Lua_Safe_Call_Void();
	self->AEAttack(attacker, distance, slot_id, count, is_from_spell, attack_rounds);
}

void Lua_EntityList::AreaSpell(Lua_Mob caster, Lua_Mob center, uint16 spell_id)
{
	Lua_Safe_Call_Void();
	self->AESpell(caster, center, spell_id);
}

void Lua_EntityList::AreaSpell(Lua_Mob caster, Lua_Mob center, uint16 spell_id, bool affect_caster)
{
	Lua_Safe_Call_Void();
	self->AESpell(caster, center, spell_id, affect_caster);
}

void Lua_EntityList::AreaSpell(Lua_Mob caster, Lua_Mob center, uint16 spell_id, bool affect_caster, int16 resist_adjust)
{
	Lua_Safe_Call_Void();
	self->AESpell(caster, center, spell_id, affect_caster, resist_adjust);
}

void Lua_EntityList::AreaSpell(Lua_Mob caster, Lua_Mob center, uint16 spell_id, bool affect_caster, int16 resist_adjust, int max_targets)
{
	Lua_Safe_Call_Void();
	self->AESpell(caster, center, spell_id, affect_caster, resist_adjust, &max_targets);
}

void Lua_EntityList::AreaTaunt(Lua_Client caster)
{
	Lua_Safe_Call_Void();
	self->AETaunt(caster);
}

void Lua_EntityList::AreaTaunt(Lua_Client caster, float range)
{
	Lua_Safe_Call_Void();
	self->AETaunt(caster, range);
}

void Lua_EntityList::AreaTaunt(Lua_Client caster, float range, int bonus_hate)
{
	Lua_Safe_Call_Void();
	self->AETaunt(caster, range, bonus_hate);
}

void Lua_EntityList::MassGroupBuff(Lua_Mob caster, Lua_Mob center, uint16 spell_id)
{
	Lua_Safe_Call_Void();
	self->MassGroupBuff(caster, center, spell_id);
}

void Lua_EntityList::MassGroupBuff(Lua_Mob caster, Lua_Mob center, uint16 spell_id, bool affect_caster)
{
	Lua_Safe_Call_Void();
	self->MassGroupBuff(caster, center, spell_id, affect_caster);
}

Lua_NPC_List Lua_EntityList::GetNPCsByExcludedIDs(luabind::adl::object table)
{
	Lua_Safe_Call_Class(Lua_NPC_List);
	Lua_NPC_List ret;

	if (luabind::type(table) != LUA_TTABLE) {
		return ret;
	}

	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);

		std::vector<uint32> ids;

		int index = 1;
		while (luabind::type(table[index]) != LUA_TNIL) {
			ids.emplace_back(luabind::object_cast<uint32>(table[index]));
			index++;
		}

		const auto& l = self->GetExcludedNPCsByIDs(ids);

		for (const auto& e : l) {
			ret.entries.emplace_back(Lua_NPC(e));
		}
	}

	return ret;
}

Lua_NPC_List Lua_EntityList::GetNPCsByIDs(luabind::adl::object table)
{
	Lua_Safe_Call_Class(Lua_NPC_List);
	Lua_NPC_List ret;

	if (luabind::type(table) != LUA_TTABLE) {
		return ret;
	}

	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);

		std::vector<uint32> ids;

		int index = 1;
		while (luabind::type(table[index]) != LUA_TNIL) {
			ids.emplace_back(luabind::object_cast<uint32>(table[index]));
			index++;
		}

		const auto& l = self->GetNPCsByIDs(ids);

		for (const auto& e : l) {
			ret.entries.emplace_back(Lua_NPC(e));
		}
	}

	return ret;
}

luabind::scope lua_register_entity_list() {
	return luabind::class_<Lua_EntityList>("EntityList")
	.def(luabind::constructor<>())
	.property("null", &Lua_EntityList::Null)
	.property("valid", &Lua_EntityList::Valid)
	.def("AreaAttack", (void(Lua_EntityList::*)(Lua_Mob, float))&Lua_EntityList::AreaAttack)
	.def("AreaAttack", (void(Lua_EntityList::*)(Lua_Mob, float, int16))&Lua_EntityList::AreaAttack)
	.def("AreaAttack", (void(Lua_EntityList::*)(Lua_Mob, float, int16, int))&Lua_EntityList::AreaAttack)
	.def("AreaAttack", (void(Lua_EntityList::*)(Lua_Mob, float, int16, int, bool))&Lua_EntityList::AreaAttack)
	.def("AreaAttack", (void(Lua_EntityList::*)(Lua_Mob, float, int16, int, bool, int))&Lua_EntityList::AreaAttack)
	.def("AreaSpell", (void(Lua_EntityList::*)(Lua_Mob, Lua_Mob, uint16))&Lua_EntityList::AreaSpell)
	.def("AreaSpell", (void(Lua_EntityList::*)(Lua_Mob, Lua_Mob, uint16, bool))&Lua_EntityList::AreaSpell)
	.def("AreaSpell", (void(Lua_EntityList::*)(Lua_Mob, Lua_Mob, uint16, bool, int16))&Lua_EntityList::AreaSpell)
	.def("AreaSpell", (void(Lua_EntityList::*)(Lua_Mob, Lua_Mob, uint16, bool, int16, int))&Lua_EntityList::AreaSpell)
	.def("AreaTaunt", (void(Lua_EntityList::*)(Lua_Client))&Lua_EntityList::AreaTaunt)
	.def("AreaTaunt", (void(Lua_EntityList::*)(Lua_Client, float))&Lua_EntityList::AreaTaunt)
	.def("AreaTaunt", (void(Lua_EntityList::*)(Lua_Client, float, int))&Lua_EntityList::AreaTaunt)
	.def("CanAddHateForMob", (bool(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::CanAddHateForMob)
	.def("ChannelMessage", (void(Lua_EntityList::*)(Lua_Mob, int, uint8, const char*))&Lua_EntityList::ChannelMessage)
	.def("ClearClientPetitionQueue", (void(Lua_EntityList::*)(void))&Lua_EntityList::ClearClientPetitionQueue)
	.def("ClearFeignAggro", (void(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::ClearFeignAggro)
	.def("DeleteNPCCorpses", (uint32(Lua_EntityList::*)(void))&Lua_EntityList::DeleteNPCCorpses)
	.def("DeletePlayerCorpses", (uint32(Lua_EntityList::*)(void))&Lua_EntityList::DeletePlayerCorpses)
	.def("DoubleAggro", (void(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::DoubleAggro)
	.def("Fighting", (bool(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::Fighting)
	.def("FilteredMessageClose", &Lua_EntityList::FilteredMessageClose)
	.def("FindDoor", (Lua_Door(Lua_EntityList::*)(uint32))&Lua_EntityList::FindDoor)
	.def("GetBotByID", (Lua_Bot(Lua_EntityList::*)(uint32))&Lua_EntityList::GetBotByID)
	.def("GetBotByName", (Lua_Bot(Lua_EntityList::*)(std::string))&Lua_EntityList::GetBotByName)
	.def("GetBotList", (Lua_Bot_List(Lua_EntityList::*)(void))&Lua_EntityList::GetBotList)
	.def("GetBotOwnerByBotEntityID", (Lua_Client(Lua_EntityList::*)(uint32))&Lua_EntityList::GetBotOwnerByBotEntityID)
	.def("GetBotOwnerByBotID", (Lua_Client(Lua_EntityList::*)(uint32))&Lua_EntityList::GetBotOwnerByBotID)
	.def("GetBotListByCharacterID", (Lua_Bot_List(Lua_EntityList::*)(uint32))&Lua_EntityList::GetBotListByCharacterID)
	.def("GetBotListByCharacterID", (Lua_Bot_List(Lua_EntityList::*)(uint32,uint8))&Lua_EntityList::GetBotListByCharacterID)
	.def("GetBotListByClientName", (Lua_Bot_List(Lua_EntityList::*)(std::string))&Lua_EntityList::GetBotListByClientName)
	.def("GetBotListByClientName", (Lua_Bot_List(Lua_EntityList::*)(std::string,uint8))&Lua_EntityList::GetBotListByClientName)
	.def("GetClientByAccID", (Lua_Client(Lua_EntityList::*)(uint32))&Lua_EntityList::GetClientByAccID)
	.def("GetClientByCharID", (Lua_Client(Lua_EntityList::*)(uint32))&Lua_EntityList::GetClientByCharID)
	.def("GetClientByID", (Lua_Client(Lua_EntityList::*)(int))&Lua_EntityList::GetClientByID)
	.def("GetClientByName", (Lua_Client(Lua_EntityList::*)(const char*))&Lua_EntityList::GetClientByName)
	.def("GetClientByWID", (Lua_Client(Lua_EntityList::*)(uint32))&Lua_EntityList::GetClientByWID)
	.def("GetClientList", (Lua_Client_List(Lua_EntityList::*)(void))&Lua_EntityList::GetClientList)
	.def("GetCloseMobList", (Lua_Mob_List(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::GetCloseMobList)
	.def("GetCloseMobList", (Lua_Mob_List(Lua_EntityList::*)(Lua_Mob,float))&Lua_EntityList::GetCloseMobList)
	.def("GetCorpseByID", (Lua_Corpse(Lua_EntityList::*)(int))&Lua_EntityList::GetCorpseByID)
	.def("GetCorpseByName", (Lua_Corpse(Lua_EntityList::*)(const char*))&Lua_EntityList::GetCorpseByName)
	.def("GetCorpseByOwner", (Lua_Corpse(Lua_EntityList::*)(Lua_Client))&Lua_EntityList::GetCorpseByOwner)
	.def("GetCorpseList", (Lua_Corpse_List(Lua_EntityList::*)(void))&Lua_EntityList::GetCorpseList)
	.def("GetDoorsByDBID", (Lua_Door(Lua_EntityList::*)(uint32))&Lua_EntityList::GetDoorsByDBID)
	.def("GetDoorsByDoorID", (Lua_Door(Lua_EntityList::*)(uint32))&Lua_EntityList::GetDoorsByDoorID)
	.def("GetDoorsByID", (Lua_Door(Lua_EntityList::*)(int))&Lua_EntityList::GetDoorsByID)
	.def("GetDoorsList", (Lua_Doors_List(Lua_EntityList::*)(void))&Lua_EntityList::GetDoorsList)
	.def("GetGroupByClient", (Lua_Group(Lua_EntityList::*)(Lua_Client))&Lua_EntityList::GetGroupByClient)
	.def("GetGroupByID", (Lua_Group(Lua_EntityList::*)(int))&Lua_EntityList::GetGroupByID)
	.def("GetGroupByLeaderName", (Lua_Group(Lua_EntityList::*)(const char*))&Lua_EntityList::GetGroupByLeaderName)
	.def("GetGroupByMob", (Lua_Group(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::GetGroupByMob)
	.def("GetMob", (Lua_Mob(Lua_EntityList::*)(const char*))&Lua_EntityList::GetMob)
	.def("GetMob", (Lua_Mob(Lua_EntityList::*)(int))&Lua_EntityList::GetMob)
	.def("GetMobByNpcTypeID", (Lua_Mob(Lua_EntityList::*)(int))&Lua_EntityList::GetMobByNpcTypeID)
	.def("GetMobID", (Lua_Mob(Lua_EntityList::*)(int))&Lua_EntityList::GetMobID)
	.def("GetMobList", (Lua_Mob_List(Lua_EntityList::*)(void))&Lua_EntityList::GetMobList)
	.def("GetNPCByID", (Lua_NPC(Lua_EntityList::*)(int))&Lua_EntityList::GetNPCByID)
	.def("GetNPCByNPCTypeID", (Lua_NPC(Lua_EntityList::*)(int))&Lua_EntityList::GetNPCByNPCTypeID)
	.def("GetNPCBySpawnID", (Lua_NPC(Lua_EntityList::*)(int))&Lua_EntityList::GetNPCBySpawnID)
	.def("GetNPCList", (Lua_NPC_List(Lua_EntityList::*)(void))&Lua_EntityList::GetNPCList)
	.def("GetNPCsByExcludedIDs", (Lua_NPC_List(Lua_EntityList::*)(luabind::adl::object))&Lua_EntityList::GetNPCsByExcludedIDs)
	.def("GetNPCsByIDs", (Lua_NPC_List(Lua_EntityList::*)(luabind::adl::object))&Lua_EntityList::GetNPCsByIDs)
	.def("GetObjectByDBID", (Lua_Object(Lua_EntityList::*)(uint32))&Lua_EntityList::GetObjectByDBID)
	.def("GetObjectByID", (Lua_Object(Lua_EntityList::*)(int))&Lua_EntityList::GetObjectByID)
	.def("GetObjectList", (Lua_Object_List(Lua_EntityList::*)(void))&Lua_EntityList::GetObjectList)
	.def("GetRaidByClient", (Lua_Raid(Lua_EntityList::*)(Lua_Client))&Lua_EntityList::GetRaidByClient)
	.def("GetRaidByID", (Lua_Raid(Lua_EntityList::*)(int))&Lua_EntityList::GetRaidByID)
	.def("GetRandomBot", (Lua_Bot(Lua_EntityList::*)(void))&Lua_EntityList::GetRandomBot)
	.def("GetRandomBot", (Lua_Bot(Lua_EntityList::*)(float,float,float,float))&Lua_EntityList::GetRandomBot)
	.def("GetRandomBot", (Lua_Bot(Lua_EntityList::*)(float,float,float,float,Lua_Bot))&Lua_EntityList::GetRandomBot)
	.def("GetRandomClient", (Lua_Client(Lua_EntityList::*)(void))&Lua_EntityList::GetRandomClient)
	.def("GetRandomClient", (Lua_Client(Lua_EntityList::*)(float,float,float,float))&Lua_EntityList::GetRandomClient)
	.def("GetRandomClient", (Lua_Client(Lua_EntityList::*)(float,float,float,float,Lua_Client))&Lua_EntityList::GetRandomClient)
	.def("GetRandomMob", (Lua_Mob(Lua_EntityList::*)(void))&Lua_EntityList::GetRandomMob)
	.def("GetRandomMob", (Lua_Mob(Lua_EntityList::*)(float,float,float,float))&Lua_EntityList::GetRandomMob)
	.def("GetRandomMob", (Lua_Mob(Lua_EntityList::*)(float,float,float,float,Lua_Mob))&Lua_EntityList::GetRandomMob)
	.def("GetRandomNPC", (Lua_NPC(Lua_EntityList::*)(void))&Lua_EntityList::GetRandomNPC)
	.def("GetRandomNPC", (Lua_NPC(Lua_EntityList::*)(float,float,float,float))&Lua_EntityList::GetRandomNPC)
	.def("GetRandomNPC", (Lua_NPC(Lua_EntityList::*)(float,float,float,float,Lua_NPC))&Lua_EntityList::GetRandomNPC)
	.def("GetShuffledClientList", (Lua_Client_List(Lua_EntityList::*)(void))&Lua_EntityList::GetShuffledClientList)
	.def("GetSpawnByID", (Lua_Spawn(Lua_EntityList::*)(uint32))&Lua_EntityList::GetSpawnByID)
	.def("GetSpawnList", (Lua_Spawn_List(Lua_EntityList::*)(void))&Lua_EntityList::GetSpawnList)
	.def("HalveAggro", (void(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::HalveAggro)
	.def("IsMobSpawnedByNpcTypeID", (bool(Lua_EntityList::*)(int))&Lua_EntityList::IsMobSpawnedByNpcTypeID)
	.def("MakeNameUnique", (std::string(Lua_EntityList::*)(const char*))&Lua_EntityList::MakeNameUnique)
	.def("Marquee", (void(Lua_EntityList::*)(uint32, std::string))&Lua_EntityList::Marquee)
	.def("Marquee", (void(Lua_EntityList::*)(uint32, std::string, uint32))&Lua_EntityList::Marquee)
	.def("Marquee", (void(Lua_EntityList::*)(uint32, uint32, uint32, uint32, uint32, std::string))&Lua_EntityList::Marquee)
	.def("MassGroupBuff", (void(Lua_EntityList::*)(Lua_Mob, Lua_Mob, uint16))&Lua_EntityList::MassGroupBuff)
	.def("MassGroupBuff", (void(Lua_EntityList::*)(Lua_Mob, Lua_Mob, uint16, bool))&Lua_EntityList::MassGroupBuff)
	.def("Message", (void(Lua_EntityList::*)(uint32, uint32, const char*))&Lua_EntityList::Message)
	.def("MessageClose", (void(Lua_EntityList::*)(Lua_Mob, bool, float, uint32, const char*))&Lua_EntityList::MessageClose)
	.def("MessageGroup", (void(Lua_EntityList::*)(Lua_Mob, bool, uint32, const char*))&Lua_EntityList::MessageGroup)
	.def("MessageStatus", (void(Lua_EntityList::*)(uint32, uint32, uint32, const char*))&Lua_EntityList::MessageStatus)
	.def("OpenDoorsNear", (void(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::OpenDoorsNear)
	.def("RemoveFromHateLists", (void(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::RemoveFromHateLists)
	.def("RemoveFromHateLists", (void(Lua_EntityList::*)(Lua_Mob, bool))&Lua_EntityList::RemoveFromHateLists)
	.def("RemoveFromTargets", (void(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::RemoveFromTargets)
	.def("RemoveFromTargets", (void(Lua_EntityList::*)(Lua_Mob, bool))&Lua_EntityList::RemoveFromTargets)
	.def("RemoveNumbers", (std::string(Lua_EntityList::*)(const char*))&Lua_EntityList::RemoveNumbers)
	.def("ReplaceWithTarget", (void(Lua_EntityList::*)(Lua_Mob, Lua_Mob))&Lua_EntityList::ReplaceWithTarget)
	.def("SignalAllBotsByOwnerCharacterID", (void(Lua_EntityList::*)(uint32, int))&Lua_EntityList::SignalAllBotsByOwnerCharacterID)
	.def("SignalAllBotsByOwnerName", (void(Lua_EntityList::*)(std::string, int))&Lua_EntityList::SignalAllBotsByOwnerName)
	.def("SignalAllClients", (void(Lua_EntityList::*)(int))&Lua_EntityList::SignalAllClients)
	.def("SignalBotByBotID", (void(Lua_EntityList::*)(uint32, int))&Lua_EntityList::SignalBotByBotID)
	.def("SignalBotByBotName", (void(Lua_EntityList::*)(std::string, int))&Lua_EntityList::SignalBotByBotName)
	.def("SignalMobsByNPCID", (void(Lua_EntityList::*)(uint32, int))&Lua_EntityList::SignalMobsByNPCID);
}

luabind::scope lua_register_mob_list() {
	return luabind::class_<Lua_Mob_List>("MobList")
	.def_readwrite("entries", &Lua_Mob_List::entries, luabind::return_stl_iterator);
}

luabind::scope lua_register_client_list() {
	return luabind::class_<Lua_Client_List>("ClientList")
	.def_readwrite("entries", &Lua_Client_List::entries, luabind::return_stl_iterator);
}

luabind::scope lua_register_bot_list() {
	return luabind::class_<Lua_Bot_List>("BotList")
	.def_readwrite("entries", &Lua_Bot_List::entries, luabind::return_stl_iterator);
}

luabind::scope lua_register_npc_list() {
	return luabind::class_<Lua_NPC_List>("NPCList")
	.def_readwrite("entries", &Lua_NPC_List::entries, luabind::return_stl_iterator);
}

luabind::scope lua_register_corpse_list() {
	return luabind::class_<Lua_Corpse_List>("CorpseList")
	.def_readwrite("entries", &Lua_Corpse_List::entries, luabind::return_stl_iterator);
}

luabind::scope lua_register_object_list() {
	return luabind::class_<Lua_Object_List>("ObjectList")
	.def_readwrite("entries", &Lua_Object_List::entries, luabind::return_stl_iterator);
}

luabind::scope lua_register_door_list() {
	return luabind::class_<Lua_Doors_List>("DoorList")
	.def_readwrite("entries", &Lua_Doors_List::entries, luabind::return_stl_iterator);
}

luabind::scope lua_register_spawn_list() {
	return luabind::class_<Lua_Spawn_List>("SpawnList")
	.def_readwrite("entries", &Lua_Spawn_List::entries, luabind::return_stl_iterator);
}

#endif
