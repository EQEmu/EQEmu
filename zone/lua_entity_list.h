#pragma once

#ifdef LUA_EQEMU

#include "zone/lua_ptr.h"

class EntityList;
class Lua_Mob;
class Lua_Client;
class Lua_Bot;
class Lua_Merc;
class Lua_NPC;
class Lua_Door;
class Lua_Corpse;
class Lua_Object;
class Lua_Group;
class Lua_Raid;
class Lua_Spawn;
struct Lua_Mob_List;
struct Lua_Client_List;
struct Lua_Bot_List;
struct Lua_NPC_List;
struct Lua_Corpse_List;
struct Lua_Object_List;
struct Lua_Doors_List;
struct Lua_Spawn_List;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_entity_list();
luabind::scope lua_register_mob_list();
luabind::scope lua_register_client_list();
luabind::scope lua_register_npc_list();
luabind::scope lua_register_corpse_list();
luabind::scope lua_register_object_list();
luabind::scope lua_register_door_list();
luabind::scope lua_register_spawn_list();
luabind::scope lua_register_bot_list();

class Lua_EntityList : public Lua_Ptr<EntityList>
{
	typedef EntityList NativeType;
public:
	Lua_EntityList() : Lua_Ptr(nullptr) { }
	Lua_EntityList(EntityList *d) : Lua_Ptr(d) { }
	virtual ~Lua_EntityList() { }

	operator EntityList*() {
		return reinterpret_cast<EntityList*>(GetLuaPtrData());
	}

	Lua_Mob GetMobID(int id);
	Lua_Mob GetMob(const char *name);
	Lua_Mob GetMob(int id);
	Lua_Mob GetMobByNpcTypeID(int npc_type);
	bool IsMobSpawnedByNpcTypeID(int npc_type);
	Lua_NPC GetNPCByID(int id);
	Lua_NPC GetNPCByNPCTypeID(int npc_type);
	Lua_NPC GetNPCBySpawnID(uint32 spawn_id);
	Lua_Client GetClientByName(const char *name);
	Lua_Client GetClientByAccID(uint32 acct_id);
	Lua_Client GetClientByID(int id);
	Lua_Client GetClientByCharID(uint32 char_id);
	Lua_Client GetClientByWID(uint32 wid);
	Lua_Object GetObjectByID(int id);
	Lua_Object GetObjectByDBID(uint32 db_id);
	Lua_Door GetDoorsByID(int id);
	Lua_Door GetDoorsByDBID(uint32 db_id);
	Lua_Door GetDoorsByDoorID(uint32 door_id);
	Lua_Door FindDoor(uint32 id);
	Lua_Group GetGroupByMob(Lua_Mob mob);
	Lua_Group GetGroupByClient(Lua_Client client);
	Lua_Group GetGroupByID(int id);
	Lua_Group GetGroupByLeaderName(const char *name);
	Lua_Raid GetRaidByID(int id);
	Lua_Raid GetRaidByClient(Lua_Client client);
	Lua_Corpse GetCorpseByOwner(Lua_Client client);
	Lua_Corpse GetCorpseByID(int id);
	Lua_Corpse GetCorpseByName(const char *name);
	Lua_Spawn GetSpawnByID(uint32 id);
	void ClearClientPetitionQueue();
	bool CanAddHateForMob(Lua_Mob p);
	void Marquee(uint32 type, std::string message);
	void Marquee(uint32 type, std::string message, uint32 duration);
	void Marquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string message);
	void Message(uint32 guild_dbid, uint32 type, const char *message);
	void MessageStatus(uint32 guild_dbid, int min_status, uint32 type, const char *message);
	void MessageClose(Lua_Mob sender, bool skip_sender, float dist, uint32 type, const char *message);
	void FilteredMessageClose(Lua_Mob sender, bool skip_sender, float dist, uint32 type, int filter, const char *message);
	void RemoveFromTargets(Lua_Mob mob);
	void RemoveFromTargets(Lua_Mob mob, bool RemoveFromXTargets);
	void ReplaceWithTarget(Lua_Mob target, Lua_Mob new_target);
	void OpenDoorsNear(Lua_Mob opener);
	std::string MakeNameUnique(const char *name);
	std::string RemoveNumbers(const char *name);
	void SignalMobsByNPCID(uint32 npc_id, int signal_id);
	uint32 DeleteNPCCorpses();
	uint32 DeletePlayerCorpses();
	void HalveAggro(Lua_Mob who);
	void DoubleAggro(Lua_Mob who);
	void ClearFeignAggro(Lua_Mob who);
	bool Fighting(Lua_Mob who);
	void RemoveFromHateLists(Lua_Mob who);
	void RemoveFromHateLists(Lua_Mob who, bool set_to_one);
	void MessageGroup(Lua_Mob who, bool skip_close, uint32 type, const char *message);
	Lua_Client GetRandomClient();
	Lua_Client GetRandomClient(float x, float y, float z, float distance);
	Lua_Client GetRandomClient(float x, float y, float z, float distance, Lua_Client exclude_client);
	Lua_Mob GetRandomMob();
	Lua_Mob GetRandomMob(float x, float y, float z, float distance);
	Lua_Mob GetRandomMob(float x, float y, float z, float distance, Lua_Mob exclude_mob);
	Lua_NPC GetRandomNPC();
	Lua_NPC GetRandomNPC(float x, float y, float z, float distance);
	Lua_NPC GetRandomNPC(float x, float y, float z, float distance, Lua_NPC exclude_npc);
	Lua_Mob_List GetMobList();
	Lua_Client_List GetClientList();
	Lua_Client_List GetShuffledClientList();
	Lua_NPC_List GetNPCList();
	Lua_Corpse_List GetCorpseList();
	Lua_Object_List GetObjectList();
	Lua_Doors_List GetDoorsList();
	Lua_Spawn_List GetSpawnList();
	void SignalAllClients(int signal_id);
	void ChannelMessage(Lua_Mob from, int channel_num, uint8 language, const char *message);
	Lua_Bot GetBotByID(uint32 bot_id);
	Lua_Bot GetBotByName(std::string bot_name);
	Lua_Client GetBotOwnerByBotEntityID(uint32 entity_id);
	Lua_Client GetBotOwnerByBotID(uint32 bot_id);
	Lua_Bot_List GetBotList();
	Lua_Bot_List GetBotListByCharacterID(uint32 character_id);
	Lua_Bot_List GetBotListByCharacterID(uint32 character_id, uint8 class_id);
	Lua_Bot_List GetBotListByClientName(std::string client_name);
	Lua_Bot_List GetBotListByClientName(std::string client_name, uint8 class_id);
	Lua_Bot GetRandomBot();
	Lua_Bot GetRandomBot(float x, float y, float z, float distance);
	Lua_Bot GetRandomBot(float x, float y, float z, float distance, Lua_Bot exclude_bot);
	void SignalAllBotsByOwnerCharacterID(uint32 character_id, int signal_id);
	void SignalAllBotsByOwnerName(std::string owner_name, int signal_id);
	void SignalBotByBotID(uint32 bot_id, int signal_id);
	void SignalBotByBotName(std::string bot_name, int signal_id);
	Lua_Mob_List GetCloseMobList(Lua_Mob mob);
	Lua_Mob_List GetCloseMobList(Lua_Mob mob, float distance);
	Lua_Mob_List GetCloseMobList(Lua_Mob mob, float distance, bool ignore_self);
	void AreaAttack(Lua_Mob attacker, float distance);
	void AreaAttack(Lua_Mob attacker, float distance, int16 slot_id);
	void AreaAttack(Lua_Mob attacker, float distance, int16 slot_id, int count);
	void AreaAttack(Lua_Mob attacker, float distance, int16 slot_id, int count, bool is_from_spell);
	void AreaAttack(Lua_Mob attacker, float distance, int16 slot_id, int count, bool is_from_spell, int attack_rounds);
	void AreaSpell(Lua_Mob caster, Lua_Mob center, uint16 spell_id);
	void AreaSpell(Lua_Mob caster, Lua_Mob center, uint16 spell_id, bool affect_caster);
	void AreaSpell(Lua_Mob caster, Lua_Mob center, uint16 spell_id, bool affect_caster, int16 resist_adjust);
	void AreaSpell(Lua_Mob caster, Lua_Mob center, uint16 spell_id, bool affect_caster, int16 resist_adjust, int max_targets);
	void AreaTaunt(Lua_Client caster);
	void AreaTaunt(Lua_Client caster, float range);
	void AreaTaunt(Lua_Client caster, float range, int bonus_hate);
	void MassGroupBuff(Lua_Mob caster, Lua_Mob center, uint16 spell_id);
	void MassGroupBuff(Lua_Mob caster, Lua_Mob center, uint16 spell_id, bool affect_caster);
	Lua_NPC_List GetNPCsByIDs(luabind::adl::object npc_ids);
	Lua_NPC_List GetNPCsByExcludedIDs(luabind::adl::object npc_ids);
};

#endif // LUA_EQEMU