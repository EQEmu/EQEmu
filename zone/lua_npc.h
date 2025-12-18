#pragma once

#ifdef LUA_EQEMU

#include "zone/lua_mob.h"

class NPC;
class Lua_Mob;
class Lua_NPC;
class Lua_Client;
struct Lua_NPC_Loot_List;
class Lua_Inventory;
class Lua_Spawn;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_npc();
luabind::scope lua_register_npc_loot_list();

class Lua_NPC : public Lua_Mob
{
	typedef NPC NativeType;
public:
	Lua_NPC() { SetLuaPtrData(nullptr); }
	Lua_NPC(NPC *d) { SetLuaPtrData(reinterpret_cast<Entity*>(d)); }
	virtual ~Lua_NPC() { }

	operator NPC*() {
		return reinterpret_cast<NPC*>(GetLuaPtrData());
	}

	void Signal(int signal_id);
	int CheckNPCFactionAlly(int faction);
	void AddItem(int item_id, int charges);
	void AddItem(int item_id, int charges, bool equip);
	void AddItem(int item_id, int charges, bool equip, int aug1);
	void AddItem(int item_id, int charges, bool equip, int aug1, int aug2);
	void AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3);
	void AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3, int aug4);
	void AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3, int aug4, int aug5);
	void AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3, int aug4, int aug5, int aug6);
	void AddLootTable();
	void AddLootTable(int id);
	void RemoveItem(int item_id);
	void RemoveItem(int item_id, int quantity);
	void RemoveItem(int item_id, int quantity, int slot);
	void ClearLootItems();
	void AddLootCash(uint32 copper, uint32 silver, uint32 gold, uint32 platinum);
	void RemoveLootCash();
	int CountLoot();
	int GetLoottableID();
	uint32 GetCopper();
	uint32 GetSilver();
	uint32 GetGold();
	uint32 GetPlatinum();
	void SetCopper(uint32 amt);
	void SetSilver(uint32 amt);
	void SetGold(uint32 amt);
	void SetPlatinum(uint32 amt);
	void SetGrid(int grid);
	void SetSaveWaypoint(int wp);
	void SetSp2(int sg2);
	int GetWaypointMax();
	int GetGrid();
	uint32 GetSp2();
	int GetNPCFactionID();
	int GetPrimaryFaction();
	int64 GetNPCHate(Lua_Mob ent);
	bool IsOnHatelist(Lua_Mob ent);
	void SetNPCFactionID(int id);
	uint32 GetMaxDMG();
	uint32 GetMinDMG();
	bool IsAnimal();
	int GetPetSpellID();
	void SetPetSpellID(int id);
	uint32 GetMaxDamage(int level);
	void SetTaunting(bool t);
	bool IsTaunting();
	void PickPocket(Lua_Client thief);
	void StartSwarmTimer(uint32 duration);
	void DoClassAttacks(Lua_Mob target);
	int GetMaxWp();
	void DisplayWaypointInfo(Lua_Client client);
	void CalculateNewWaypoint();
	void AssignWaypoints(int grid);
	void SetWaypointPause();
	void UpdateWaypoint(int wp);
	void StopWandering();
	void ResumeWandering();
	void PauseWandering(int pause_time);
	void MoveTo(float x, float y, float z, float h, bool save);
	void NextGuardPosition();
	void SaveGuardSpot();
	void SaveGuardSpot(bool clear);
	void SaveGuardSpot(float x, float y, float z, float heading);
	bool IsGuarding();
	void AI_SetRoambox(float dist, float max_x, float min_x, float max_y, float min_y);
	void AI_SetRoambox(float dist, float max_x, float min_x, float max_y, float min_y, uint32 delay, uint32 mindelay);
	void SetFollowID(int id);
	void SetFollowDistance(int dist);
	void SetFollowCanRun(bool v);
	int GetFollowID();
	int GetFollowDistance();
	bool GetFollowCanRun();
	uint32 GetNPCSpellsID();
	int GetSpawnPointID();
	float GetSpawnPointX();
	float GetSpawnPointY();
	float GetSpawnPointZ();
	float GetSpawnPointH();
	float GetGuardPointX();
	float GetGuardPointY();
	float GetGuardPointZ();
	void SetPrimSkill(int skill_id);
	void SetSecSkill(int skill_id);
	int GetPrimSkill();
	int GetSecSkill();
	int GetSwarmOwner();
	int GetSwarmTarget();
	void SetSwarmTarget(int target);
	void ModifyNPCStat(std::string stat, std::string value);
	void AddAISpell(int priority, int spell_id, int type, int mana_cost, int recast_delay, int resist_adjust);
	void AddAISpell(int priority, int spell_id, int type, int mana_cost, int recast_delay, int resist_adjust, int min_hp, int max_hp);
	void RemoveAISpell(int spell_id);
	void SetSpellFocusDMG(int focus);
	void SetSpellFocusHeal(int focus);
	int GetSpellFocusDMG();
	int GetSpellFocusHeal();
	float GetSlowMitigation();
	float GetAttackSpeed();
	int GetAttackDelay();
	int GetAccuracyRating();
	int GetSpawnKillCount();
	int GetScore();
	void MerchantOpenShop();
	void MerchantCloseShop();
	int GetRawAC();
	int GetAvoidanceRating();
	void SetSimpleRoamBox(float box_size);
	void SetSimpleRoamBox(float box_size, float move_distance);
	void SetSimpleRoamBox(float box_size, float move_distance, int move_delay);
	void RecalculateSkills();
	void ReloadSpells();
	bool IsRaidTarget();
	bool IsRareSpawn();
	void ChangeLastName(std::string last_name);
	void ClearLastName();
	bool HasItem(uint32 item_id);
	uint32 CountItem(uint32 item_id);
	uint32 GetLootItemIDBySlot(uint16 loot_slot);
	uint16 GetFirstLootSlotByItemID(uint32 item_id);
	float GetHealScale();
	float GetSpellScale();
	Lua_NPC_Loot_List GetLootList(lua_State* L);
	void AddAISpellEffect(int spell_effect_id, int base_value, int limit_value, int max_value);
	void RemoveAISpellEffect(int spell_effect_id);
	bool HasAISpellEffect(int spell_effect_id);
	float GetNPCStat(std::string stat);
	void SendPayload(int payload_id);
	void SendPayload(int payload_id, std::string payload_value);
	bool GetKeepsSoldItems();
	void SetKeepsSoldItems(bool keeps_sold_items);
	bool IsLDoNTrapped();
	void SetLDoNTrapped(bool is_trapped);
	uint8 GetLDoNTrapType();
	void SetLDoNTrapType(uint8 trap_type);
	uint16 GetLDoNTrapSpellID();
	void SetLDoNTrapSpellID(uint16 spell_id);
	bool IsLDoNLocked();
	void SetLDoNLocked(bool is_locked);
	uint16 GetLDoNLockedSkill();
	void SetLDoNLockedSkill(uint16 skill_value);
	bool IsLDoNTrapDetected();
	void SetLDoNTrapDetected(bool is_detected);
	void ScaleNPC(uint8 npc_level);
	void ScaleNPC(uint8 npc_level, bool override_special_abilities);
	bool IsUnderwaterOnly();
	bool HasSpecialAbilities();
	void DeleteBucket(std::string bucket_name);
	std::string GetBucket(std::string bucket_name);
	std::string GetBucketExpires(std::string bucket_name);
	std::string GetBucketRemaining(std::string bucket_name);
	void SetBucket(std::string bucket_name, std::string bucket_value);
	void SetBucket(std::string bucket_name, std::string bucket_value, std::string expiration);
	bool GetNPCAggro();
	void SetNPCAggro(bool in_npc_aggro);
	uint32 GetNPCSpellsEffectsID();
	void DescribeSpecialAbilities(Lua_Client c);
	bool IsMultiQuestEnabled();
	void MultiQuestEnable();
	bool LuaCheckHandin(
		Lua_Client c,
		luabind::adl::object handin_table,
		luabind::adl::object required_table,
		luabind::adl::object items_table
	);
	void ReturnHandinItems(Lua_Client c);
	Lua_Spawn GetSpawn(lua_State* L);
	bool IsResumedFromZoneSuspend();
	void SetNPCTintIndex(uint32 id);
	uint32 GetNPCTintIndex();

};

#endif // LUA_EQEMU
