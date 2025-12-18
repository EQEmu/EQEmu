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

#include "common/discord/discord.h"
#include "common/eqtime.h"
#include "common/linked_list.h"
#include "common/random.h"
#include "common/repositories/base_data_repository.h"
#include "common/repositories/dynamic_zone_templates_repository.h"
#include "common/repositories/faction_association_repository.h"
#include "common/repositories/grid_entries_repository.h"
#include "common/repositories/grid_repository.h"
#include "common/repositories/lootdrop_entries_repository.h"
#include "common/repositories/lootdrop_repository.h"
#include "common/repositories/loottable_entries_repository.h"
#include "common/repositories/loottable_repository.h"
#include "common/repositories/npc_faction_entries_repository.h"
#include "common/repositories/npc_faction_repository.h"
#include "common/repositories/player_titlesets_repository.h"
#include "common/repositories/skill_caps_repository.h"
#include "common/repositories/spawn2_disabled_repository.h"
#include "common/repositories/zone_points_repository.h"
#include "common/repositories/zone_state_spawns_repository.h"
#include "common/rulesys.h"
#include "common/strings.h"
#include "common/types.h"
#include "common/zone_store.h"
#include "zone/aa_ability.h"
#include "zone/global_loot_manager.h"
#include "zone/pathfinder_interface.h"
#include "zone/qglobals.h"
#include "zone/queryserv.h"
#include "zone/spawn2.h"
#include "zone/spawngroup.h"
#include "zone/zonedb.h"

struct EXPModifier
{
	float aa_modifier;
	float exp_modifier;
};

class DynamicZone;

struct ZonePoint {
	float  x;
	float  y;
	float  z;
	float  heading;
	uint16 number;
	float  target_x;
	float  target_y;
	float  target_z;
	float  target_heading;
	uint16 target_zone_id;
	int32  target_zone_instance;
	uint32 client_version_mask;
	bool   is_virtual;
	int    height;
	int    width;
};

struct ZoneClientAuth_Struct {
	uint32 ip;            // client's IP address
	uint32 wid;        // client's WorldID#
	uint32 accid;
	int16  admin;
	uint32 charid;
	uint32 lsid;
	bool   tellsoff;
	char   charname[64];
	char   lskey[30];
	bool   stale;
};

struct ZoneEXPModInfo {
	float ExpMod;
	float AAExpMod;
};

class Client;
class Map;
class Mob;
class WaterMap;
extern EntityList entity_list;
struct NPCType;
struct ServerZoneIncomingClient_Struct;
class MobMovementManager;

class Zone {
public:
	static bool Bootup(uint32 iZoneID, uint32 iInstanceID, bool is_static = false);
	void Shutdown(bool quiet = false);

	Zone(uint32 in_zoneid, uint32 in_instanceid, const char *in_short_name);
	~Zone();

	AA::Ability *GetAlternateAdvancementAbility(int id);
	AA::Ability *GetAlternateAdvancementAbilityByRank(int rank_id);
	AA::Rank *GetAlternateAdvancementRank(int rank_id);
	bool is_zone_time_localized;
	bool quest_idle_override;
	bool IsIdleWhenEmpty() const;
	void SetIdleWhenEmpty(bool idle_when_empty);
	uint32 GetSecondsBeforeIdle() const;
	void SetSecondsBeforeIdle(uint32 seconds_before_idle);
	bool AggroLimitReached() { return (aggroedmobs > 10) ? true : false; }
	bool AllowMercs() const { return (allow_mercs); }
	bool CanBind() const { return (can_bind); }
	bool CanCastOutdoor() const { return (can_castoutdoor); } //qadar
	bool CanDoCombat() const { return (can_combat); }
	bool CanLevitate() const { return (can_levitate); } // Magoth78
	bool IsWaterZone(float z);
	bool Depop(bool StartSpawnTimer = false);
	bool did_adventure_actions;
	bool GetAuth(
		uint32 iIP,
		const char *iCharName,
		uint32 *oWID = 0,
		uint32 *oAccID = 0,
		uint32 *oCharID = 0,
		int16 *oStatus = 0,
		char *oLSKey = 0,
		bool *oTellsOff = 0
	);
	bool HasGraveyard();
	bool HasWeather();
	bool Init(bool is_static);
	bool IsCity() const { return (is_city); }
	bool IsHotzone() const { return (is_hotzone); }
	bool IsLoaded();
	bool IsPVPZone() { return pvpzone; }
	bool IsSpellBlocked(uint32 spell_id, const glm::vec3 &location);
	bool IsUCSServerAvailable() { return m_ucss_available; }
	bool IsZone(uint32 zone_id, uint16 instance_id) const;
	bool LoadGroundSpawns();
	bool LoadZoneCFG(const char *filename, uint16 instance_version);
	bool LoadZoneObjects();
	bool IsSpecialBindLocation(const glm::vec4& location);
	bool Process();
	bool SaveZoneCFG();
	bool DoesAlternateCurrencyExist(uint32 currency_id);
	void DisableRespawnTimers();

	char *adv_data;

	const char *GetSpellBlockedMessage(uint32 spell_id, const glm::vec3 &location);

	EQ::Random random;
	EQTime     zone_time;

	ZonePoint *
	GetClosestZonePoint(const glm::vec3 &location, const char *to_name, Client *client, float max_distance = 40000.0f);

	inline bool BuffTimersSuspended() const { return newzone_data.suspend_buffs != 0; };
	inline bool HasMap() { return zonemap != nullptr; }
	inline bool HasWaterMap() { return watermap != nullptr; }
	inline bool InstantGrids() { return (!initgrids_timer.Enabled()); }
	inline bool IsStaticZone() { return staticzone; }
	inline const bool IsInstancePersistent() const { return pers_instance; }
	inline const char *GetFileName() { return file_name; }
	inline const char *GetLongName() { return long_name; }
	inline const char *GetShortName() { return short_name; }
	inline const uint8 GetZoneType() const { return zone_type; }
	inline const uint16 GetInstanceVersion() const { return instanceversion; }
	inline const uint32 &GetMaxClients() { return m_max_clients; }
	inline const uint32 &graveyard_id() { return m_graveyard_id; }
	inline const uint32 &graveyard_zoneid() { return pgraveyard_zoneid; }
	inline const uint32 GetInstanceID() const { return instanceid; }
	inline const uint32 GetZoneID() const { return zoneid; }
	inline glm::vec4 GetSafePoint() { return m_safe_points; }
	inline glm::vec4 GetGraveyardPoint() { return m_graveyard; }
	inline std::vector<int> GetGlobalLootTables(NPC *mob) const { return m_global_loot.GetGlobalLootTables(mob); }
	inline Timer *GetInstanceTimer() { return Instance_Timer; }
	inline void AddGlobalLootEntry(GlobalLootEntry &in) { return m_global_loot.AddEntry(in); }
	inline void SetZoneHasCurrentTime(bool time) { zone_has_current_time = time; }
	inline void ShowNPCGlobalLoot(Client *c, NPC *t) { m_global_loot.ShowNPCGlobalLoot(c, t); }
	inline void ShowZoneGlobalLoot(Client *c) { m_global_loot.ShowZoneGlobalLoot(c); }
	int GetZoneTotalBlockedSpells() { return zone_total_blocked_spells; }
	int SaveTempItem(uint32 merchantid, uint32 npcid, uint32 item, int32 charges, bool sold = false);
	int32 MobsAggroCount() { return aggroedmobs; }
	DynamicZone *GetDynamicZone();

	bool ClearVariables();
	bool DeleteVariable(const std::string& variable_name);
	std::string GetVariable(const std::string& variable_name);
	std::vector<std::string> GetVariables();
	void SetVariable(const std::string& variable_name, const std::string& variable_value);
	bool VariableExists(const std::string& variable_name);

	IPathfinder                                   *pathing;
	std::vector<NPC_Emote_Struct *>               npc_emote_list;
	LinkedList<Spawn2 *>                          spawn2_list;
	LinkedList<ZonePoint *>                       zone_point_list;
	std::vector<ZonePointsRepository::ZonePoints> virtual_zone_point_list;

	Map                   *zonemap;
	MercTemplate *GetMercTemplate(uint32 template_id);
	NewZone_Struct        newzone_data;
	QGlobalCache *CreateQGlobals()
	{
		qGlobals = new QGlobalCache();
		return qGlobals;
	}
	QGlobalCache *GetQGlobals() { return qGlobals; }
	SpawnConditionManager spawn_conditions;
	SpawnGroupList        spawn_group_list;

	std::list<AltCurrencyDefinition_Struct>          AlternateCurrencies;
	std::list<InternalVeteranReward>                 VeteranRewards;
	std::map<uint32, LDoNTrapTemplate *>             ldon_trap_list;
	std::map<uint32, MercTemplate>                   merc_templates;
	std::map<uint32, NPCType *>                      merctable;
	std::map<uint32, NPCType *>                      npctable;
	std::map<uint32, std::list<LDoNTrapTemplate *> > ldon_trap_entry_list;
	std::map<uint32, std::list<MerchantList> >       merchanttable;
	std::map<uint32, std::list<MercSpellEntry> >     merc_spells_list;
	std::map<uint32, std::list<MercStanceInfo> >     merc_stance_list;
	std::map<uint32, std::list<TempMerchantList> >   tmpmerchanttable;
	std::map<uint32, std::string>                    adventure_entry_list_flavor;
	std::map<uint32, ZoneEXPModInfo>                 level_exp_mod;

	std::pair<AA::Ability *, AA::Rank *> GetAlternateAdvancementAbilityAndRank(int id, int points_spent);

	std::unordered_map<int, std::unique_ptr<AA::Ability>> aa_abilities;
	std::unordered_map<int, std::unique_ptr<AA::Rank>>    aa_ranks;

	std::vector<GridRepository::Grid>               zone_grids;
	std::vector<GridEntriesRepository::GridEntries> zone_grid_entries;

	std::unordered_map<uint32, std::unique_ptr<DynamicZone>> dynamic_zone_cache;
	std::unordered_map<uint32, DynamicZoneTemplatesRepository::DynamicZoneTemplates> dz_template_cache;

	std::unordered_map<uint32, EXPModifier> exp_modifiers;

	std::vector<uint32> discovered_items;

	std::map<std::string, std::string> m_zone_variables;

	time_t weather_timer;
	Timer  spawn2_timer;
	Timer  hot_reload_timer;

	uint8  weather_intensity;
	uint8  zone_weather;
	uint8  loglevelvar;
	uint8  lootvar;
	uint8  merchantvar;
	uint8  tradevar;
	uint32 numzonepoints;
	uint32 CountAuth();
	uint32 CountSpawn2();
	uint32 GetSpawnKillCount(uint32 in_spawnid);
	uint32 GetTempMerchantQuantity(uint32 NPCID, uint32 Slot);

	uint32 GetCurrencyID(uint32 item_id);
	uint32 GetCurrencyItemID(uint32 currency_id);

	std::string GetAAName(int aa_id);

	inline bool IsRaining() { return zone_weather == EQ::constants::WeatherTypes::Raining; }
	inline bool IsSnowing() { return zone_weather == EQ::constants::WeatherTypes::Snowing; }

	std::string GetZoneDescription();
	void SendReloadMessage(std::string reload_type);

	void ClearEXPModifier(Client* c);
	void ClearEXPModifierByCharacterID(const uint32 character_id);
	float GetAAEXPModifier(Client* c);
	float GetAAEXPModifierByCharacterID(const uint32 character_id);
	float GetEXPModifier(Client* c);
	float GetEXPModifierByCharacterID(const uint32 character_id);
	void SetAAEXPModifier(Client* c, float aa_modifier);
	void SetAAEXPModifierByCharacterID(const uint32 character_id, float aa_modifier);
	void SetEXPModifier(Client* c, float exp_modifier);
	void SetEXPModifierByCharacterID(const uint32 character_id, float exp_modifier);

	void AddAggroMob() { aggroedmobs++; }
	void AddAuth(ServerZoneIncomingClient_Struct *szic);
	void ChangeWeather();
	void ClearBlockedSpells();
	void ClearNPCTypeCache(int id);
	void CalculateNpcUpdateDistanceSpread();
	void DelAggroMob() { aggroedmobs--; }
	void DeleteQGlobal(std::string name, uint32 npcID, uint32 charID, uint32 zoneID);
	void Despawn(uint32 spawngroupID);
	void DoAdventureActions();
	void DoAdventureAssassinationCountIncrease();
	void DoAdventureCountIncrease();
	void LoadMerchants();
	void GetTimeSync();
	void LoadAdventureFlavor();
	void LoadAlternateAdvancement();
	void LoadAlternateCurrencies();
	void LoadDynamicZoneTemplates();
	void LoadZoneBlockedSpells();
	void LoadLDoNTrapEntries();
	void LoadLDoNTraps();
	void LoadLevelEXPMods();
	void LoadGrids();
	void LoadMercenarySpells();
	void LoadMercenaryTemplates();
	void LoadNewMerchantData(uint32 merchantid);
	void LoadNPCEmotes(std::vector<NPC_Emote_Struct*>* v);
	void LoadTempMerchantData();
	void LoadVeteranRewards();
	void LoadZoneDoors();
	void ReloadStaticData();
	void RemoveAuth(const char *iCharName, const char *iLSKey);
	void RemoveAuth(uint32 lsid);
	void Repop(bool is_forced = false);
	void RequestUCSServerStatus();
	void ResetAuth();
	void SetDate(uint16 year, uint8 month, uint8 day, uint8 hour, uint8 minute);
	void SetInstanceTimer(uint32 new_duration);
	void SetStaticZone(bool sz) { staticzone = sz; }
	void SetTime(uint8 hour, uint8 minute, bool update_world = true);
	void SetUCSServerAvailable(bool ucss_available, uint32 update_timestamp);
	void SpawnConditionChanged(const SpawnCondition &c, int16 old_value);
	void StartShutdownTimer(uint32 set_time = (RuleI(Zone, AutoShutdownDelay)));
	void ResetShutdownTimer();
	void StopShutdownTimer();
	void UpdateQGlobal(uint32 qid, QGlobal newGlobal);
	void weatherSend(Client *client = nullptr);
	void ClearSpawnTimers();

	bool IsQuestHotReloadQueued() const;
	void SetQuestHotReloadQueued(bool in_quest_hot_reload_queued);

	bool CompareDataBucket(uint8 comparison_type, const std::string& bucket, const std::string& value);

	WaterMap *watermap;
	ZonePoint *GetClosestZonePoint(const glm::vec3 &location, uint32 to, Client *client, float max_distance = 40000.0f);
	ZonePoint *GetClosestZonePointWithoutZone(float x, float y, float z, Client *client, float max_distance = 40000.0f);

	Timer GetInitgridsTimer();
	uint32 GetInstanceTimeRemaining() const;
	void SetInstanceTimeRemaining(uint32 instance_time_remaining);

	inline bool GetSaveZoneState() const { return m_save_zone_state; }
	inline void SetSaveZoneState(bool save_state) { m_save_zone_state = save_state; }

	/**
	 * GMSay Callback for LogSys
	 *
	 * @param log_category
	 * @param message
	 */
	static void GMSayHookCallBackProcess(uint16 log_category, const char *func, std::string message)
	{
		// we don't want to loop up with chat messages
		if (message.find("OP_SpecialMesg") != std::string::npos) {
			return;
		}

		/**
		 * Cut messages down to 4000 max to prevent client crash
		 */
		if (!message.empty()) {
			message = message.substr(0, 4000);
		}

		/**
		 * Replace Occurrences of % or MessageStatus will crash
		 */
		Strings::FindReplace(message, std::string("%"), std::string("."));

		if (message.find('\n') != std::string::npos) {
			auto message_split = Strings::Split(message, '\n');
			entity_list.MessageStatus(
				0,
				AccountStatus::QuestTroupe,
				EQEmuLogSys::Instance()->GetGMSayColorFromCategory(log_category),
				message_split[0].c_str()
			);

			for (size_t iter = 1; iter < message_split.size(); ++iter) {
				entity_list.MessageStatus(
					0,
					AccountStatus::QuestTroupe,
					EQEmuLogSys::Instance()->GetGMSayColorFromCategory(log_category),
					fmt::format(
						"--- {}",
						message_split[iter]
					).c_str()
				);
			}
		}
		else {
			entity_list.MessageStatus(
				0,
				AccountStatus::QuestTroupe,
				EQEmuLogSys::Instance()->GetGMSayColorFromCategory(log_category),
				fmt::format("[{}] [{}] {}", Logs::LogCategoryName[log_category], func, message).c_str()
			);
		}
	}

	static void SendDiscordMessage(int webhook_id, const std::string& message);
	static void SendDiscordMessage(const std::string& webhook_name, const std::string& message);
	static void DiscordWebhookMessageHandler(uint16 log_category, int webhook_id, const std::string &message)
	{
		std::string message_prefix;
		if (!EQEmuLogSys::Instance()->origination_info.zone_short_name.empty()) {
			message_prefix = fmt::format(
				"[**{}**] **Zone** [**{}**] ",
				Logs::LogCategoryName[log_category],
				EQEmuLogSys::Instance()->origination_info.zone_short_name
			);
		}

		SendDiscordMessage(webhook_id, message_prefix + Discord::FormatDiscordMessage(log_category, message));
	};

	double GetClientUpdateRange() const { return m_client_update_range; }

	void SetIsHotzone(bool is_hotzone);

	void ReloadContentFlags();

	void LoadNPCFaction(const uint32 npc_faction_id);
	void LoadNPCFactions(const std::vector<uint32>& npc_faction_ids);
	void ClearNPCFactions();
	void ReloadNPCFactions();
	NpcFactionRepository::NpcFaction* GetNPCFaction(const uint32 npc_faction_id);
	std::vector<NpcFactionEntriesRepository::NpcFactionEntries> GetNPCFactionEntries(const uint32 npc_faction_id) const;

	void LoadNPCFactionAssociation(const uint32 npc_faction_id);
	void LoadNPCFactionAssociations(const std::vector<uint32>& npc_faction_ids);
	void LoadFactionAssociation(const uint32 faction_id);
	void LoadFactionAssociations(const std::vector<uint32>& faction_ids);
	void ClearFactionAssociations();
	void ReloadFactionAssociations();
	FactionAssociationRepository::FactionAssociation* GetFactionAssociation(const uint32 faction_id);

	// loot
	void LoadLootTable(const uint32 loottable_id);
	void LoadLootTables(const std::vector<uint32> in_loottable_ids);
	void LoadLootDrops(const std::vector<uint32> in_lootdrop_ids);
	void ClearLootTables();
	void ReloadLootTables();
	LoottableRepository::Loottable *GetLootTable(const uint32 loottable_id);
	std::vector<LoottableEntriesRepository::LoottableEntries> GetLootTableEntries(const uint32 loottable_id) const;
	LootdropRepository::Lootdrop GetLootdrop(const uint32 lootdrop_id) const;
	std::vector<LootdropEntriesRepository::LootdropEntries> GetLootdropEntries(const uint32 lootdrop_id) const;

	// Base Data
	inline void ClearBaseData() { m_base_data.clear(); };
	BaseDataRepository::BaseData GetBaseData(uint8 level, uint8 class_id);
	void LoadBaseData();
	void ReloadBaseData();

	// data buckets
	std::string GetBucket(const std::string& bucket_name);
	void SetBucket(const std::string& bucket_name, const std::string& bucket_value, const std::string& expiration = "");
	void DeleteBucket(const std::string& bucket_name);
	std::string GetBucketExpires(const std::string& bucket_name);
	std::string GetBucketRemaining(const std::string& bucket_name);
	inline void SetZoneServerId(uint32 id) { m_zone_server_id = id; }
	inline uint32 GetZoneServerId() const { return m_zone_server_id; }

	// zone state
	bool LoadZoneVariablesState();
	bool LoadZoneState(
		std::unordered_map<uint32, uint32> spawn_times,
		std::vector<Spawn2DisabledRepository::Spawn2Disabled> disabled_spawns
	);
	void SaveZoneState();
	static void ClearZoneState(uint32 zone_id, uint32 instance_id);
	void ReloadMaps();

	void Signal(int signal_id);
	void SendPayload(int payload_id, std::string payload_value);

	struct PausedZoneTimer {
		std::string name;
		uint32      remaining_time;
	};

	uint32 GetTimerDuration(std::string name);
	uint32 GetTimerRemainingTime(std::string name);
	bool HasTimer(std::string name);
	bool IsPausedTimer(std::string name);
	void PauseTimer(std::string name);
	void ResumeTimer(std::string name);
	void SetTimer(std::string name, uint32 duration);
	void StopTimer(std::string name);
	void StopAllTimers();
	std::vector<std::string> GetPausedTimers();
	std::vector<std::string> GetTimers();

private:
	bool      allow_mercs;
	bool      can_bind;
	bool      can_castoutdoor;
	bool      can_combat;
	bool      can_levitate;
	bool      is_city;
	bool      is_hotzone;
	bool      pers_instance;
	bool      pvpzone;
	bool      m_ucss_available;
	bool      staticzone;
	bool      zone_has_current_time;
	bool      quest_hot_reload_queued;
	double    m_client_update_range;
	char      *long_name;
	char      *map_name;
	char      *short_name;
	char      file_name[32];
	glm::vec4 m_safe_points;
	glm::vec4 m_graveyard;
	int       default_ruleset;
	int       zone_total_blocked_spells;
	int       npc_position_update_distance;
	int32     aggroedmobs;
	uint8     zone_type;
	uint16    instanceversion;
	uint32    instanceid;
	uint32    instance_time_remaining;
	uint32    m_graveyard_id, pgraveyard_zoneid;
	uint32    m_max_clients;
	uint32    zoneid;
	uint32    m_last_ucss_update;
	bool      m_idle_when_empty;
	uint32    m_seconds_before_idle;
	bool      m_save_zone_state;

	GlobalLootManager                   m_global_loot;
	LinkedList<ZoneClientAuth_Struct *> client_auth_list;
	MobMovementManager                  *mMovementManager;
	QGlobalCache                        *qGlobals;
	Timer                               *Instance_Shutdown_Timer;
	Timer                               *Instance_Timer;
	Timer                               *Instance_Warning_timer;
	Timer                               *Weather_Timer;
	Timer                               autoshutdown_timer;
	Timer                               clientauth_timer;
	Timer                               initgrids_timer;
	Timer                               qglobal_purge_timer;
	ZoneSpellsBlocked                   *blocked_spells;

	// Factions
	std::vector<NpcFactionRepository::NpcFaction>                 m_npc_factions         = { };
	std::vector<NpcFactionEntriesRepository::NpcFactionEntries>   m_npc_faction_entries  = { };
	std::vector<FactionAssociationRepository::FactionAssociation> m_faction_associations = { };

	// loot
	std::vector<LoottableRepository::Loottable>               m_loottables        = {};
	std::vector<LoottableEntriesRepository::LoottableEntries> m_loottable_entries = {};
	std::vector<LootdropRepository::Lootdrop>                 m_lootdrops         = {};
	std::vector<LootdropEntriesRepository::LootdropEntries>   m_lootdrop_entries  = {};

	// Base Data
	std::vector<BaseDataRepository::BaseData> m_base_data = { };

	uint32_t m_zone_server_id = 0;

	class ZoneTimer {
	public:
		inline ZoneTimer(std::string _name, uint32 duration)
			: name(_name), timer_(duration) { timer_.Start(duration, false); }
		std::string name;
		Timer       timer_;
	};

	std::vector<ZoneTimer> zone_timers;
	std::vector<PausedZoneTimer> paused_zone_timers;
	std::deque<int> m_zone_signals;
};
