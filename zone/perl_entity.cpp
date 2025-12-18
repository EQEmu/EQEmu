#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "common/rulesys.h"
#include "common/say_link.h"
#include "zone/embperl.h"
#include "zone/entity.h"
#include "zone/mob.h"

#include <list>

Mob* Perl_EntityList_GetMobID(EntityList* self, uint16_t mob_id) // @categories Script Utility
{
	return self->GetMobID(mob_id);
}

Mob* Perl_EntityList_GetMob(EntityList* self, const char* name)
{
	return self->GetMob(name);
}

Mob* Perl_EntityList_GetMobByID(EntityList* self, uint16_t mob_id) // @categories Script Utility
{
	return self->GetMob(mob_id);
}

Mob* Perl_EntityList_GetMobByNpcTypeID(EntityList* self, uint32_t npc_type_id) // @categories Script Utility
{
	return self->GetMobByNpcTypeID(npc_type_id);
}

bool Perl_EntityList_IsMobSpawnedByNpcTypeID(EntityList* self, uint32_t npc_type_id) // @categories Script Utility
{
	return self->IsMobSpawnedByNpcTypeID(npc_type_id);
}

NPC* Perl_EntityList_GetNPCByID(EntityList* self, uint16_t id) // @categories Script Utility
{
	return self->GetNPCByID(id);
}

NPC* Perl_EntityList_GetNPCByNPCTypeID(EntityList* self, uint32_t npc_id) // @categories Script Utility
{
	return self->GetNPCByNPCTypeID(npc_id);
}

NPC* Perl_EntityList_GetNPCBySpawnID(EntityList* self, uint32_t spawn_id) // @categories Script Utility, Spawns
{
	return self->GetNPCBySpawnID(spawn_id);
}

Client* Perl_EntityList_GetClientByName(EntityList* self, const char* name) // @categories Account and Character, Script Utility
{
	return self->GetClientByName(name);
}

Client* Perl_EntityList_GetClientByAccID(EntityList* self, uint32_t account_id) // @categories Account and Character, Script Utility
{
	return self->GetClientByAccID(account_id);
}

Client* Perl_EntityList_GetClientByID(EntityList* self, uint16_t client_id) // @categories Account and Character, Script Utility
{
	return self->GetClientByID(client_id);
}

Client* Perl_EntityList_GetClientByCharID(EntityList* self, uint32_t character_id) // @categories Account and Character, Script Utility
{
	return self->GetClientByCharID(character_id);
}

Client* Perl_EntityList_GetClientByWID(EntityList* self, uint32_t wid) // @categories Account and Character, Script Utility
{
	return self->GetClientByWID(wid);
}

Object* Perl_EntityList_GetObjectByDBID(EntityList* self, uint32_t database_id) // @categories Script Utility, Objects
{
	return self->GetObjectByDBID(database_id);
}

Object* Perl_EntityList_GetObjectByID(EntityList* self, uint32_t entity_id) // @categories Script Utility, Objects
{
	return self->GetObjectByID(entity_id);
}

Doors* Perl_EntityList_GetDoorsByDBID(EntityList* self, uint32_t database_id) // @categories Script Utility, Doors
{
	return self->GetDoorsByDBID(database_id);
}

Doors* Perl_EntityList_GetDoorsByDoorID(EntityList* self, uint32_t door_id) // @categories Script Utility, Doors
{
	return self->GetDoorsByDoorID(door_id);
}

Doors* Perl_EntityList_GetDoorsByID(EntityList* self, uint32_t entity_id) // @categories Script Utility, Doors
{
	return self->GetDoorsByID(entity_id);
}

Doors* Perl_EntityList_FindDoor(EntityList* self, uint32_t door_id) // @categories Script Utility, Doors
{
	return self->FindDoor(door_id);
}

Group* Perl_EntityList_GetGroupByMob(EntityList* self, Mob* mob) // @categories Account and Character, Script Utility, Group
{
	return self->GetGroupByMob(mob);
}

Group* Perl_EntityList_GetGroupByClient(EntityList* self, Client* client) // @categories Account and Character, Script Utility, Group
{
	return self->GetGroupByClient(client);
}

Group* Perl_EntityList_GetGroupByID(EntityList* self, uint32_t id) // @categories Account and Character, Script Utility, Group
{
	return self->GetGroupByID(id);
}

Group* Perl_EntityList_GetGroupByLeaderName(EntityList* self, const char* leader_name) // @categories Account and Character, Script Utility, Group
{
	return self->GetGroupByLeaderName(leader_name);
}

Raid* Perl_EntityList_GetRaidByID(EntityList* self, uint32_t id) // @categories Script Utility, Raid
{
	return self->GetRaidByID(id);
}

Raid* Perl_EntityList_GetRaidByClient(EntityList* self, Client* client) // @categories Account and Character, Script Utility, Raid
{
	return self->GetRaidByClient(client);
}

Corpse* Perl_EntityList_GetCorpseByOwner(EntityList* self, Client* client) // @categories Script Utility, Corpse
{
	return self->GetCorpseByOwner(client);
}

Corpse* Perl_EntityList_GetCorpseByID(EntityList* self, uint16_t id) // @categories Script Utility, Corpse
{
	return self->GetCorpseByID(id);
}

Corpse* Perl_EntityList_GetCorpseByName(EntityList* self, const char* name) // @categories Script Utility, Corpse
{
	return self->GetCorpseByName(name);
}

void Perl_EntityList_ClearClientPetitionQueue(EntityList* self) // @categories Script Utility
{
	self->ClearClientPetitionQueue();
}

bool Perl_EntityList_CanAddHateForMob(EntityList* self, Mob* target) // @categories Script Utility, Hate and Aggro
{
	return self->CanAddHateForMob(target);
}

void Perl_EntityList_Clear(EntityList* self) // @categories Script Utility
{
	self->Clear();
}

bool Perl_EntityList_RemoveMob(EntityList* self, uint16_t delete_id) // @categories Script Utility
{
	return self->RemoveMob(delete_id);
}

bool Perl_EntityList_RemoveClient(EntityList* self, uint16_t delete_id) // @categories Account and Character, Script Utility
{
	return self->RemoveClient(delete_id);
}

bool Perl_EntityList_RemoveNPC(EntityList* self, uint16_t delete_id) // @categories Script Utility
{
	return self->RemoveNPC(delete_id);
}

bool Perl_EntityList_RemoveGroup(EntityList* self, uint32_t delete_id) // @categories Script Utility, Group
{
	return self->RemoveGroup(delete_id);
}

bool Perl_EntityList_RemoveCorpse(EntityList* self, uint16_t delete_id) // @categories Corpse
{
	return self->RemoveCorpse(delete_id);
}

bool Perl_EntityList_RemoveDoor(EntityList* self, uint16_t delete_id) // @categories Doors
{
	return self->RemoveDoor(delete_id);
}

bool Perl_EntityList_RemoveTrap(EntityList* self, uint16_t delete_id) // @categories Script Utility
{
	return self->RemoveTrap(delete_id);
}

bool Perl_EntityList_RemoveObject(EntityList* self, uint16_t delete_id) // @categories Script Utility, Objects
{
	return self->RemoveObject(delete_id);
}

void Perl_EntityList_RemoveAllMobs(EntityList* self) // @categories Script Utility
{
	self->RemoveAllMobs();
}

void Perl_EntityList_RemoveAllClients(EntityList* self) // @categories Script Utility
{
	self->RemoveAllClients();
}

void Perl_EntityList_RemoveAllNPCs(EntityList* self) // @categories Script Utility
{
	self->RemoveAllNPCs();
}

void Perl_EntityList_RemoveAllGroups(EntityList* self) // @categories Group
{
	self->RemoveAllGroups();
}

void Perl_EntityList_RemoveAllCorpses(EntityList* self) // @categories Corpse
{
	self->RemoveAllCorpses();
}

void Perl_EntityList_RemoveAllDoors(EntityList* self) // @categories Doors
{
	self->RemoveAllDoors();
}

void Perl_EntityList_RemoveAllTraps(EntityList* self) // @categories Script Utility
{
	self->RemoveAllTraps();
}

void Perl_EntityList_RemoveAllObjects(EntityList* self) // @categories Objects
{
	self->RemoveAllObjects();
}

void Perl_EntityList_Message(EntityList* self, uint32 guild_id, uint32 color_type, const char* message) // @categories Script Utility
{
	self->Message(guild_id, color_type, message);
}

void Perl_EntityList_MessageStatus(EntityList* self, uint32 guild_id, int to_minstatus, uint32 color_type, const char* message) // @categories Script Utility
{
	self->MessageStatus(guild_id, to_minstatus, color_type, message);
}

void Perl_EntityList_MessageClose(EntityList* self, Mob* sender, bool skip_sender, float distance, uint32 color_type, const char* message)
{
	if (RuleB(Chat, AutoInjectSaylinksToClientMessage))
	{
		std::string new_message = EQ::SayLinkEngine::InjectSaylinksIfNotExist(message);
		self->MessageClose(sender, skip_sender, distance, color_type, new_message.c_str());
	}
	else
	{
		self->MessageClose(sender, skip_sender, distance, color_type, message);
	}
}

void Perl_EntityList_RemoveFromTargets(EntityList* self, Mob* mob) // @categories Script Utility
{
	self->RemoveFromTargets(mob);
}

void Perl_EntityList_ReplaceWithTarget(EntityList* self, Mob* old_mob, Mob* new_target) // @categories Script Utility
{
	self->ReplaceWithTarget(old_mob, new_target);
}

void Perl_EntityList_OpenDoorsNear(EntityList* self, Mob* opener) // @categories Script Utility, Doors
{
	self->OpenDoorsNear(opener);
}

std::string Perl_EntityList_MakeNameUnique(EntityList* self, char* name) // @categories Script Utility
{
	char buf[64] = {0};
	strn0cpy(buf, name, sizeof(buf));
	return self->MakeNameUnique(buf); // todo: this function is unsafe
}

std::string Perl_EntityList_RemoveNumbers(EntityList* self, char* name)
{
	char buf[64] = {0};
	strn0cpy(buf, name, sizeof(buf));
	return EntityList::RemoveNumbers(buf); // todo: this function is unsafe
}

void Perl_EntityList_SignalMobsByNPCID(EntityList* self, uint32 npc_type_id, int signal_id) // @categories Script Utility
{
	self->SignalMobsByNPCID(npc_type_id, signal_id);
}

void Perl_EntityList_RemoveEntity(EntityList* self, uint16_t id) // @categories Script Utility
{
	self->RemoveEntity(id);
}

uint32_t Perl_EntityList_DeleteNPCCorpses(EntityList* self) // @categories Corpse
{
	return self->DeleteNPCCorpses();
}

uint32_t Perl_EntityList_DeletePlayerCorpses(EntityList* self) // @categories Account and Character, Corpse
{
	return self->DeletePlayerCorpses();
}

void Perl_EntityList_HalveAggro(EntityList* self, Mob* who) // @categories Script Utility, Hate and Aggro
{
	self->HalveAggro(who);
}

void Perl_EntityList_DoubleAggro(EntityList* self, Mob* who) // @categories Script Utility
{
	self->DoubleAggro(who);
}

void Perl_EntityList_ClearFeignAggro(EntityList* self, Mob* target) // @categories Script Utility
{
	self->ClearFeignAggro(target);
}

bool Perl_EntityList_Fighting(EntityList* self, Mob* target) // @categories Script Utility
{
	return self->Fighting(target);
}

void Perl_EntityList_RemoveFromHateLists(EntityList* self, Mob* mob) // @categories Script Utility, Hate and Aggro
{
	self->RemoveFromHateLists(mob);
}

void Perl_EntityList_RemoveFromHateLists(EntityList* self, Mob* mob, bool set_to_one) // @categories Script Utility, Hate and Aggro
{
	self->RemoveFromHateLists(mob, set_to_one);
}

void Perl_EntityList_MessageGroup(EntityList* self, Mob* sender, bool skip_close, uint32_t emote_color_type, const char* message) // @categories Script Utility, Group
{
	self->MessageGroup(sender, skip_close, emote_color_type, message);
}

Client* Perl_EntityList_GetRandomClient(EntityList* self) // @categories Account and Character, Script Utility
{
	return self->GetRandomClient();
}

Client* Perl_EntityList_GetRandomClient(EntityList* self, float x, float y, float z, float distance) // @categories Account and Character, Script Utility
{
	return self->GetRandomClient(glm::vec3(x, y, z), distance);
}

Client* Perl_EntityList_GetRandomClient(EntityList* self, float x, float y, float z, float distance, Client* exclude_client) // @categories Account and Character, Script Utility
{
	return self->GetRandomClient(glm::vec3(x, y, z), distance, exclude_client);
}

perl::array Perl_EntityList_GetMobList(EntityList* self) // @categories Script Utility
{
	perl::array result;

	std::list<Mob*> mob_list;
	entity_list.GetMobList(mob_list);
	for (Mob* entry : mob_list)
	{
		result.push_back(entry);
	}

	return result;
}

perl::array Perl_EntityList_GetClientList(EntityList* self) // @categories Account and Character, Script Utility
{
	perl::array result;

	std::list<Client*> client_list;
	entity_list.GetClientList(client_list);
	for (Client* entry : client_list)
	{
		result.push_back(entry);
	}

	return result;
}

Bot* Perl_EntityList_GetBotByID(EntityList* self, uint32_t bot_id) // @categories Script Utility, Bot
{
	return self->GetBotByBotID(bot_id);
}

Bot* Perl_EntityList_GetBotByName(EntityList* self, std::string bot_name) // @categories Script Utility, Bot
{
	return self->GetBotByBotName(bot_name);
}

Client* Perl_EntityList_GetBotOwnerByBotEntityID(EntityList* self, uint32_t entity_id) // @categories Script Utility, Bot
{
	return self->GetBotOwnerByBotEntityID(entity_id);
}

Client* Perl_EntityList_GetBotOwnerByBotID(EntityList* self, uint32_t bot_id) // @categories Script Utility, Bot
{
	return self->GetBotOwnerByBotID(bot_id);
}

perl::array Perl_EntityList_GetBotList(EntityList* self) // @categories Script Utility, Bot
{
	perl::array result;
	auto current_bot_list = self->GetBotList();
	auto it = current_bot_list.begin();

	while (it != current_bot_list.end()) {
		result.push_back(it->second);
		++it;
	}

	return result;
}

perl::array Perl_EntityList_GetBotListByCharacterID(EntityList* self, uint32_t character_id) // @categories Script Utility, Bot
{
	perl::array result;
	auto current_bot_list = self->GetBotListByCharacterID(character_id);
	for (int i = 0; i < current_bot_list.size(); ++i)
	{
		result.push_back(current_bot_list[i]);
	}
	return result;
}

perl::array Perl_EntityList_GetBotListByCharacterID(EntityList* self, uint32_t character_id, uint8_t class_id) // @categories Script Utility, Bot
{
	perl::array result;
	auto current_bot_list = self->GetBotListByCharacterID(character_id, class_id);
	for (int i = 0; i < current_bot_list.size(); ++i)
	{
		result.push_back(current_bot_list[i]);
	}
	return result;
}

perl::array Perl_EntityList_GetBotListByClientName(EntityList* self, std::string client_name) // @categories Script Utility, Bot
{
	perl::array result;
	auto current_bot_list = self->GetBotListByClientName(client_name);
	for (int i = 0; i < current_bot_list.size(); ++i)
	{
		result.push_back(current_bot_list[i]);
	}
	return result;
}

perl::array Perl_EntityList_GetBotListByClientName(EntityList* self, std::string client_name, uint8 class_id) // @categories Script Utility, Bot
{
	perl::array result;
	auto current_bot_list = self->GetBotListByClientName(client_name, class_id);
	for (int i = 0; i < current_bot_list.size(); ++i)
	{
		result.push_back(current_bot_list[i]);
	}
	return result;
}

void Perl_EntityList_SignalAllBotsByOwnerCharacterID(EntityList* self, uint32_t character_id, int signal_id) // @categories Script Utility
{
	entity_list.SignalAllBotsByOwnerCharacterID(character_id, signal_id);
}

void Perl_EntityList_SignalAllBotsByOwnerName(EntityList* self, std::string owner_name, int signal_id) // @categories Script Utility
{
	entity_list.SignalAllBotsByOwnerName(owner_name, signal_id);
}

void Perl_EntityList_SignalBotByBotID(EntityList* self, uint32_t bot_id, int signal_id) // @categories Script Utility
{
	entity_list.SignalBotByBotID(bot_id, signal_id);
}

void Perl_EntityList_SignalBotByBotName(EntityList* self, std::string bot_name, int signal_id) // @categories Script Utility
{
	entity_list.SignalBotByBotName(bot_name, signal_id);
}

perl::array Perl_EntityList_GetNPCList(EntityList* self) // @categories Script Utility
{
	perl::array result;

	std::list<NPC*> npc_list;
	entity_list.GetNPCList(npc_list);
	for (NPC* entry : npc_list)
	{
		result.push_back(entry);
	}

	return result;
}

perl::array Perl_EntityList_GetCorpseList(EntityList* self) // @categories Script Utility, Corpse
{
	perl::array result;

	std::list<Corpse*> corpse_list;
	entity_list.GetCorpseList(corpse_list);
	for (Corpse* entry : corpse_list)
	{
		result.push_back(entry);
	}

	return result;
}

perl::array Perl_EntityList_GetObjectList(EntityList* self) // @categories Script Utility, Objects
{
	perl::array result;

	std::list<Object*> object_list;
	entity_list.GetObjectList(object_list);
	for (Object* entry : object_list)
	{
		result.push_back(entry);
	}

	return result;
}

perl::array Perl_EntityList_GetDoorsList(EntityList* self) // @categories Script Utility, Doors
{
	perl::array result;

	std::list<Doors*> door_list;
	entity_list.GetDoorsList(door_list);
	for (Doors* entry : door_list)
	{
		result.push_back(entry);
	}

	return result;
}

void Perl_EntityList_SignalAllClients(EntityList* self, int signal_id) // @categories Script Utility
{
	entity_list.SignalAllClients(signal_id);
}

Mob* Perl_EntityList_GetRandomMob(EntityList* self) // @categories Account and Character, Script Utility
{
	return self->GetRandomMob();
}

Mob* Perl_EntityList_GetRandomMob(EntityList* self, float x, float y, float z, float distance) // @categories Account and Character, Script Utility
{
	return self->GetRandomMob(glm::vec3(x, y, z), distance);
}

Mob* Perl_EntityList_GetRandomMob(EntityList* self, float x, float y, float z, float distance, Mob* exclude_mob) // @categories Account and Character, Script Utility
{
	return self->GetRandomMob(glm::vec3(x, y, z), distance, exclude_mob);
}

NPC* Perl_EntityList_GetRandomNPC(EntityList* self) // @categories Account and Character, Script Utility
{
	return self->GetRandomNPC();
}

NPC* Perl_EntityList_GetRandomNPC(EntityList* self, float x, float y, float z, float distance) // @categories Account and Character, Script Utility
{
	return self->GetRandomNPC(glm::vec3(x, y, z), distance);
}

NPC* Perl_EntityList_GetRandomNPC(EntityList* self, float x, float y, float z, float distance, NPC* exclude_npc) // @categories Account and Character, Script Utility
{
	return self->GetRandomNPC(glm::vec3(x, y, z), distance, exclude_npc);
}

void Perl_EntityList_Marquee(EntityList* self, uint32 type, std::string message)
{
	self->Marquee(type, message);
}

void Perl_EntityList_Marquee(EntityList* self, uint32 type, std::string message, uint32 duration)
{
	self->Marquee(type, message, duration);
}

void Perl_EntityList_Marquee(EntityList* self, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string message)
{
	self->Marquee(type, priority, fade_in, fade_out, duration, message);
}

Bot* Perl_EntityList_GetRandomBot(EntityList* self) // @categories Bots, Script Utility
{
	return self->GetRandomBot();
}

Bot* Perl_EntityList_GetRandomBot(EntityList* self, float x, float y, float z, float distance) // @categories Bot, Script Utility
{
	return self->GetRandomBot(glm::vec3(x, y, z), distance);
}

Bot* Perl_EntityList_GetRandomBot(EntityList* self, float x, float y, float z, float distance, Bot* exclude_bot) // @categories Bot, Script Utility
{
	return self->GetRandomBot(glm::vec3(x, y, z), distance, exclude_bot);
}

perl::array Perl_EntityList_GetCloseMobList(EntityList* self, Mob* mob)
{
	perl::array result;
	const auto& l = self->GetCloseMobList(mob);
	result.reserve(l.size());
	for (const auto& e : l) {
		result.push_back(e.second);
	}

	return result;
}

perl::array Perl_EntityList_GetCloseMobList(EntityList* self, Mob* mob, float distance)
{
	perl::array result;
	for (const auto& e : self->GetCloseMobList(mob, distance)) {
		if (mob->CalculateDistance(e.second) <= distance) {
			result.push_back(e.second);
		}
	}

	return result;
}

perl::array Perl_EntityList_GetCloseMobList(EntityList* self, Mob* mob, float distance, bool ignore_self)
{
	perl::array result;
	for (const auto& e : self->GetCloseMobList(mob, distance)) {
		if (ignore_self && e.second == mob) {
			continue;
		}

		if (mob->CalculateDistance(e.second) <= distance) {
			result.push_back(e.second);
		}
	}

	return result;
}

Spawn2* Perl_EntityList_GetSpawnByID(EntityList* self, uint32 spawn_id) {
	return self->GetSpawnByID(spawn_id);
}

perl::array Perl_EntityList_GetSpawnList(EntityList* self) {
	perl::array ret;

	std::list<Spawn2*> l;

	self->GetSpawnList(l);

	for (const auto& e : l) {
		ret.push_back(e);
	}

	return ret;
}

void Perl_EntityList_AreaAttack(EntityList* self, Mob* attacker, float distance)
{
	self->AEAttack(attacker, distance);
}

void Perl_EntityList_AreaAttack(EntityList* self, Mob* attacker, float distance, int16 slot_id)
{
	self->AEAttack(attacker, distance, slot_id);
}

void Perl_EntityList_AreaAttack(EntityList* self, Mob* attacker, float distance, int16 slot_id, int count)
{
	self->AEAttack(attacker, distance, slot_id, count);
}

void Perl_EntityList_AreaAttack(EntityList* self, Mob* attacker, float distance, int16 slot_id, int count, bool is_from_spell)
{
	self->AEAttack(attacker, distance, slot_id, count, is_from_spell);
}

void Perl_EntityList_AreaAttack(EntityList* self, Mob* attacker, float distance, int16 slot_id, int count, bool is_from_spell, int attack_rounds)
{
	self->AEAttack(attacker, distance, slot_id, count, is_from_spell, attack_rounds);
}

void Perl_EntityList_AreaSpell(EntityList* self, Mob* caster, Mob* center, uint16 spell_id)
{
	self->AESpell(caster, center, spell_id);
}

void Perl_EntityList_AreaSpell(EntityList* self, Mob* caster, Mob* center, uint16 spell_id, bool affect_caster)
{
	self->AESpell(caster, center, spell_id, affect_caster);
}

void Perl_EntityList_AreaSpell(EntityList* self, Mob* caster, Mob* center, uint16 spell_id, bool affect_caster, int16 resist_adjust)
{
	self->AESpell(caster, center, spell_id, affect_caster, resist_adjust);
}

void Perl_EntityList_AreaSpell(EntityList* self, Mob* caster, Mob* center, uint16 spell_id, bool affect_caster, int16 resist_adjust, int max_targets)
{
	self->AESpell(caster, center, spell_id, affect_caster, resist_adjust, &max_targets);
}

void Perl_EntityList_AreaTaunt(EntityList* self, Client* caster)
{
	self->AETaunt(caster);
}

void Perl_EntityList_AreaTaunt(EntityList* self, Client* caster, float range)
{
	self->AETaunt(caster, range);
}

void Perl_EntityList_AreaTaunt(EntityList* self, Client* caster, float range, int bonus_hate)
{
	self->AETaunt(caster, range, bonus_hate);
}

void Perl_EntityList_MassGroupBuff(EntityList* self, Mob* caster, Mob* center, uint16 spell_id)
{
	self->MassGroupBuff(caster, center, spell_id);
}

void Perl_EntityList_MassGroupBuff(EntityList* self, Mob* caster, Mob* center, uint16 spell_id, bool affect_caster)
{
	self->MassGroupBuff(caster, center, spell_id, affect_caster);
}

perl::array Perl_EntityList_GetNPCsByExcludedIDs(EntityList* self, perl::array npc_ids)
{
	std::vector<uint32> ids;

	for (int i = 0; i < npc_ids.size(); i++) {
		ids.emplace_back(npc_ids[i]);
	}

	const auto& l = self->GetExcludedNPCsByIDs(ids);

	perl::array npcs;

	for (const auto& e : l) {
		npcs.push_back(e);
	}

	return npcs;
}

perl::array Perl_EntityList_GetNPCsByIDs(EntityList* self, perl::array npc_ids)
{
	std::vector<uint32> ids;

	for (int i = 0; i < npc_ids.size(); i++) {
		ids.emplace_back(npc_ids[i]);
	}

	const auto& l = self->GetNPCsByIDs(ids);

	perl::array npcs;

	for (const auto& e : l) {
		npcs.push_back(e);
	}

	return npcs;
}

void perl_register_entitylist()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<EntityList>("EntityList");
	package.add("AreaAttack", (void(*)(EntityList*, Mob*, float))&Perl_EntityList_AreaAttack);
	package.add("AreaAttack", (void(*)(EntityList*, Mob*, float, int16))&Perl_EntityList_AreaAttack);
	package.add("AreaAttack", (void(*)(EntityList*, Mob*, float, int16, int))&Perl_EntityList_AreaAttack);
	package.add("AreaAttack", (void(*)(EntityList*, Mob*, float, int16, int, bool))&Perl_EntityList_AreaAttack);
	package.add("AreaAttack", (void(*)(EntityList*, Mob*, float, int16, int, bool, int))&Perl_EntityList_AreaAttack);
	package.add("AreaSpell", (void(*)(EntityList*, Mob*, Mob*, uint16))&Perl_EntityList_AreaSpell);
	package.add("AreaSpell", (void(*)(EntityList*, Mob*, Mob*, uint16, bool))&Perl_EntityList_AreaSpell);
	package.add("AreaSpell", (void(*)(EntityList*, Mob*, Mob*, uint16, bool, int16))&Perl_EntityList_AreaSpell);
	package.add("AreaSpell", (void(*)(EntityList*, Mob*, Mob*, uint16, bool, int16, int))&Perl_EntityList_AreaSpell);
	package.add("AreaTaunt", (void(*)(EntityList*, Client*))&Perl_EntityList_AreaTaunt);
	package.add("AreaTaunt", (void(*)(EntityList*, Client*, float))&Perl_EntityList_AreaTaunt);
	package.add("AreaTaunt", (void(*)(EntityList*, Client*, float, int))&Perl_EntityList_AreaTaunt);
	package.add("CanAddHateForMob", &Perl_EntityList_CanAddHateForMob);
	package.add("Clear", &Perl_EntityList_Clear);
	package.add("ClearClientPetitionQueue", &Perl_EntityList_ClearClientPetitionQueue);
	package.add("ClearFeignAggro", &Perl_EntityList_ClearFeignAggro);
	package.add("DeleteNPCCorpses", &Perl_EntityList_DeleteNPCCorpses);
	package.add("DeletePlayerCorpses", &Perl_EntityList_DeletePlayerCorpses);
	package.add("DoubleAggro", &Perl_EntityList_DoubleAggro);
	package.add("Fighting", &Perl_EntityList_Fighting);
	package.add("FindDoor", &Perl_EntityList_FindDoor);
	package.add("GetBotByID", &Perl_EntityList_GetBotByID);
	package.add("GetBotByName", &Perl_EntityList_GetBotByName);
	package.add("GetBotList", &Perl_EntityList_GetBotList);
	package.add("GetBotOwnerByBotEntityID", (Client*(*)(EntityList*, uint32))&Perl_EntityList_GetBotOwnerByBotEntityID);
	package.add("GetBotOwnerByBotID", (Client*(*)(EntityList*, uint32))&Perl_EntityList_GetBotOwnerByBotID);
	package.add("GetBotListByCharacterID", (perl::array(*)(EntityList*, uint32))&Perl_EntityList_GetBotListByCharacterID);
	package.add("GetBotListByCharacterID", (perl::array(*)(EntityList*, uint32, uint8))&Perl_EntityList_GetBotListByCharacterID);
	package.add("GetBotListByClientName", (perl::array(*)(EntityList*, std::string))&Perl_EntityList_GetBotListByClientName);
	package.add("GetBotListByClientName", (perl::array(*)(EntityList*, std::string, uint8))&Perl_EntityList_GetBotListByClientName);
	package.add("GetClientByAccID", &Perl_EntityList_GetClientByAccID);
	package.add("GetClientByCharID", &Perl_EntityList_GetClientByCharID);
	package.add("GetClientByID", &Perl_EntityList_GetClientByID);
	package.add("GetClientByName", &Perl_EntityList_GetClientByName);
	package.add("GetClientByWID", &Perl_EntityList_GetClientByWID);
	package.add("GetClientList", &Perl_EntityList_GetClientList);
	package.add("GetCloseMobList", (perl::array(*)(EntityList*, Mob*))&Perl_EntityList_GetCloseMobList);
	package.add("GetCloseMobList", (perl::array(*)(EntityList*, Mob*, float))&Perl_EntityList_GetCloseMobList);
	package.add("GetCloseMobList", (perl::array(*)(EntityList*, Mob*, float, bool))&Perl_EntityList_GetCloseMobList);
	package.add("GetCorpseByID", &Perl_EntityList_GetCorpseByID);
	package.add("GetCorpseByName", &Perl_EntityList_GetCorpseByName);
	package.add("GetCorpseByOwner", &Perl_EntityList_GetCorpseByOwner);
	package.add("GetCorpseList", &Perl_EntityList_GetCorpseList);
	package.add("GetDoorsByDBID", &Perl_EntityList_GetDoorsByDBID);
	package.add("GetDoorsByDoorID", &Perl_EntityList_GetDoorsByDoorID);
	package.add("GetDoorsByID", &Perl_EntityList_GetDoorsByID);
	package.add("GetDoorsList", &Perl_EntityList_GetDoorsList);
	package.add("GetGroupByClient", &Perl_EntityList_GetGroupByClient);
	package.add("GetGroupByID", &Perl_EntityList_GetGroupByID);
	package.add("GetGroupByLeaderName", &Perl_EntityList_GetGroupByLeaderName);
	package.add("GetGroupByMob", &Perl_EntityList_GetGroupByMob);
	package.add("GetMob", &Perl_EntityList_GetMob);
	package.add("GetMobByID", &Perl_EntityList_GetMobByID);
	package.add("GetMobByNpcTypeID", &Perl_EntityList_GetMobByNpcTypeID);
	package.add("GetMobID", &Perl_EntityList_GetMobID);
	package.add("GetMobList", &Perl_EntityList_GetMobList);
	package.add("GetNPCByID", &Perl_EntityList_GetNPCByID);
	package.add("GetNPCByNPCTypeID", &Perl_EntityList_GetNPCByNPCTypeID);
	package.add("GetNPCBySpawnID", &Perl_EntityList_GetNPCBySpawnID);
	package.add("GetNPCList", &Perl_EntityList_GetNPCList);
	package.add("GetNPCsByExcludedIDs", &Perl_EntityList_GetNPCsByExcludedIDs);
	package.add("GetNPCsByIDs", &Perl_EntityList_GetNPCsByIDs);
	package.add("GetObjectByDBID", &Perl_EntityList_GetObjectByDBID);
	package.add("GetObjectByID", &Perl_EntityList_GetObjectByID);
	package.add("GetObjectList", &Perl_EntityList_GetObjectList);
	package.add("GetRaidByClient", &Perl_EntityList_GetRaidByClient);
	package.add("GetRaidByID", &Perl_EntityList_GetRaidByID);
	package.add("GetRandomBot", (Bot*(*)(EntityList*))&Perl_EntityList_GetRandomBot);
	package.add("GetRandomBot", (Bot*(*)(EntityList*, float, float, float, float))&Perl_EntityList_GetRandomBot);
	package.add("GetRandomBot", (Bot*(*)(EntityList*, float, float, float, float, Bot*))&Perl_EntityList_GetRandomBot);
	package.add("GetRandomClient", (Client*(*)(EntityList*))&Perl_EntityList_GetRandomClient);
	package.add("GetRandomClient", (Client*(*)(EntityList*, float, float, float, float))&Perl_EntityList_GetRandomClient);
	package.add("GetRandomClient", (Client*(*)(EntityList*, float, float, float, float, Client*))&Perl_EntityList_GetRandomClient);
	package.add("GetRandomMob", (Mob*(*)(EntityList*))&Perl_EntityList_GetRandomMob);
	package.add("GetRandomMob", (Mob*(*)(EntityList*, float, float, float, float))&Perl_EntityList_GetRandomMob);
	package.add("GetRandomMob", (Mob*(*)(EntityList*, float, float, float, float, Mob*))&Perl_EntityList_GetRandomMob);
	package.add("GetRandomNPC", (NPC*(*)(EntityList*))&Perl_EntityList_GetRandomNPC);
	package.add("GetRandomNPC", (NPC*(*)(EntityList*, float, float, float, float))&Perl_EntityList_GetRandomNPC);
	package.add("GetRandomNPC", (NPC*(*)(EntityList*, float, float, float, float, NPC*))&Perl_EntityList_GetRandomNPC);
	package.add("GetSpawnByID", (Spawn2*(*)(EntityList*, uint32))&Perl_EntityList_GetSpawnByID);
	package.add("GetSpawnList", (perl::array(*)(EntityList*))&Perl_EntityList_GetSpawnList);
	package.add("HalveAggro", &Perl_EntityList_HalveAggro);
	package.add("IsMobSpawnedByNpcTypeID", &Perl_EntityList_IsMobSpawnedByNpcTypeID);
	package.add("MakeNameUnique", &Perl_EntityList_MakeNameUnique);
	package.add("Marquee", (void(*)(EntityList*, uint32, std::string))&Perl_EntityList_Marquee);
	package.add("Marquee", (void(*)(EntityList*, uint32, std::string, uint32))&Perl_EntityList_Marquee);
	package.add("Marquee", (void(*)(EntityList*, uint32, uint32, uint32, uint32, uint32, std::string))&Perl_EntityList_Marquee);
	package.add("MassGroupBuff", (void(*)(EntityList*, Mob*, Mob*, uint16))&Perl_EntityList_MassGroupBuff);
	package.add("MassGroupBuff", (void(*)(EntityList*, Mob*, Mob*, uint16, bool))&Perl_EntityList_MassGroupBuff);
	package.add("Message", &Perl_EntityList_Message);
	package.add("MessageClose", &Perl_EntityList_MessageClose);
	package.add("MessageGroup", &Perl_EntityList_MessageGroup);
	package.add("MessageStatus", &Perl_EntityList_MessageStatus);
	package.add("OpenDoorsNear", &Perl_EntityList_OpenDoorsNear);
	package.add("RemoveAllClients", &Perl_EntityList_RemoveAllClients);
	package.add("RemoveAllCorpses", &Perl_EntityList_RemoveAllCorpses);
	package.add("RemoveAllDoors", &Perl_EntityList_RemoveAllDoors);
	package.add("RemoveAllGroups", &Perl_EntityList_RemoveAllGroups);
	package.add("RemoveAllMobs", &Perl_EntityList_RemoveAllMobs);
	package.add("RemoveAllNPCs", &Perl_EntityList_RemoveAllNPCs);
	package.add("RemoveAllObjects", &Perl_EntityList_RemoveAllObjects);
	package.add("RemoveAllTraps", &Perl_EntityList_RemoveAllTraps);
	package.add("RemoveClient", &Perl_EntityList_RemoveClient);
	package.add("RemoveCorpse", &Perl_EntityList_RemoveCorpse);
	package.add("RemoveDoor", &Perl_EntityList_RemoveDoor);
	package.add("RemoveEntity", &Perl_EntityList_RemoveEntity);
	package.add("RemoveFromHateLists", (void(*)(EntityList*, Mob*))&Perl_EntityList_RemoveFromHateLists);
	package.add("RemoveFromHateLists", (void(*)(EntityList*, Mob*, bool))&Perl_EntityList_RemoveFromHateLists);
	package.add("RemoveFromTargets", &Perl_EntityList_RemoveFromTargets);
	package.add("RemoveGroup", &Perl_EntityList_RemoveGroup);
	package.add("RemoveMob", &Perl_EntityList_RemoveMob);
	package.add("RemoveNPC", &Perl_EntityList_RemoveNPC);
	package.add("RemoveNumbers", &Perl_EntityList_RemoveNumbers);
	package.add("RemoveObject", &Perl_EntityList_RemoveObject);
	package.add("RemoveTrap", &Perl_EntityList_RemoveTrap);
	package.add("ReplaceWithTarget", &Perl_EntityList_ReplaceWithTarget);
	package.add("SignalAllBotsByOwnerCharacterID", &Perl_EntityList_SignalAllBotsByOwnerCharacterID);
	package.add("SignalAllBotsByOwnerName", &Perl_EntityList_SignalAllBotsByOwnerName);
	package.add("SignalAllClients", &Perl_EntityList_SignalAllClients);
	package.add("SignalBotByBotID", &Perl_EntityList_SignalBotByBotID);
	package.add("SignalBotByBotName", &Perl_EntityList_SignalBotByBotName);
	package.add("SignalMobsByNPCID", &Perl_EntityList_SignalMobsByNPCID);
}

#endif //EMBPERL_XS_CLASSES
