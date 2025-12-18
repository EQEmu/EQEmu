#include "zone_save_state.h"

#include "common/repositories/criteria/content_filter_criteria.h"
#include "common/repositories/spawn2_repository.h"
#include "zone/corpse.h"
#include "zone/npc.h"
#include "zone/zone.h"

#include "cereal/archives/json.hpp"
#include "cereal/types/map.hpp"
#include <string>

// IsZoneStateValid checks if the zone state is valid
// if these fields are all empty or zero value for an entire zone state, it's considered invalid
inline bool IsZoneStateValid(std::vector<ZoneStateSpawnsRepository::ZoneStateSpawns> &spawns)
{
	return std::any_of(
		spawns.begin(), spawns.end(), [](const auto &s) {
			return !(
				s.hp == 0 &&
				s.mana == 0 &&
				s.endurance == 0 &&
				s.loot_data.empty() &&
				s.entity_variables.empty() &&
				s.buffs.empty()
			);
		}
	);
}

inline void LoadLootStateData(Zone *zone, NPC *npc, const std::string &loot_data)
{
	LootStateData l{};

	// in the event that should never happen, we roll loot from the NPC's table
	if (loot_data.empty()) {
		LogZoneState("No loot state data found for NPC [{}], re-rolling", npc->GetNPCTypeID());
		npc->ClearLootItems();
		npc->AddLootTable();
		if (npc->DropsGlobalLoot()) {
			npc->CheckGlobalLootTables();
		}

		return;
	}

	if (!Strings::IsValidJson(loot_data)) {
		LogZoneState("Invalid JSON data for NPC [{}]", npc->GetNPCTypeID());
		return;
	}

	try {
		std::stringstream ss;
		{
			ss << loot_data;
			cereal::JSONInputArchive ar(ss);
			l.serialize(ar);
		}
	} catch (const std::exception &e) {
		LogZoneState("Failed to load loot state data for NPC [{}] [{}]", npc->GetNPCTypeID(), e.what());
		return;
	}

	// reset
	npc->RemoveLootCash();
	npc->ClearLootItems();

	// add loot
	npc->AddLootCash(l.copper, l.silver, l.gold, l.platinum);

	for (auto &e: l.entries) {
		const auto *db_item = database.GetItem(e.item_id);
		if (!db_item) {
			continue;
		}

		// dynamically added via AddItem
		if (e.lootdrop_id == 0) {
			npc->AddItem(e.item_id, e.charges, true);
			continue;
		}

		const auto entries = zone->GetLootdropEntries(e.lootdrop_id);
		if (entries.empty()) {
			continue;
		}

		LootdropEntriesRepository::LootdropEntries lootdrop_entry;

		for (auto &le: entries) {
			if (e.item_id == le.item_id) {
				lootdrop_entry = le;
				break;
			}
		}

		npc->AddLootDrop(db_item, lootdrop_entry);
	}
}

inline std::string GetLootSerialized(NPC *npc)
{
	LootStateData ls         = {};
	auto          loot_items = npc->GetLootItems(); // Assuming this returns a list of loot items
	ls.copper   = npc->GetCopper();
	ls.silver   = npc->GetSilver();
	ls.gold     = npc->GetGold();
	ls.platinum = npc->GetPlatinum();
	ls.entries.reserve(loot_items.size());

	for (auto &l: loot_items) {
		ls.entries.emplace_back(
			LootEntryStateData{
				.item_id = l->item_id,
				.lootdrop_id = l->lootdrop_id,
				.charges = l->charges,
			}
		);
	}

	try {
		std::stringstream ss;
		{
			cereal::JSONOutputArchiveSingleLine ar(ss);
			ls.serialize(ar);
		}

		return ss.str();
	} catch (const std::exception &e) {
		LogZoneState("Failed to serialize loot data for NPC [{}] [{}]", npc->GetNPCTypeID(), e.what());
		return "";
	}

	return "";
}

inline std::string GetLootSerialized(Corpse *c)
{
	LootStateData ls         = {};
	auto          loot_items = c->GetLootItems(); // Assuming this returns a list of loot items
	ls.copper   = c->GetCopper();
	ls.silver   = c->GetSilver();
	ls.gold     = c->GetGold();
	ls.platinum = c->GetPlatinum();
	ls.entries.reserve(loot_items.size());

	for (auto &l: loot_items) {
		ls.entries.emplace_back(
			LootEntryStateData{
				.item_id = l->item_id,
				.lootdrop_id = l->lootdrop_id,
			}
		);
	}

	try {
		std::stringstream ss;
		{
			cereal::JSONOutputArchiveSingleLine ar(ss);
			ls.serialize(ar);
		}

		return ss.str();
	} catch (const std::exception &e) {
		LogZoneState("Failed to serialize loot data for Corpse [{}] [{}]", c->GetID(), e.what());
		return "";
	}

	return "";
}

inline std::map<std::string, std::string> GetVariablesDeserialized(const std::string &entity_variables)
{
	std::map<std::string, std::string> deserialized_map;

	if (entity_variables.empty()) {
		return deserialized_map;
	}

	if (!Strings::IsValidJson(entity_variables)) {
		LogZoneState("Invalid JSON data for entity variables");
		return deserialized_map;
	}

	try {
		std::stringstream ss;
		{
			ss << entity_variables;
			cereal::JSONInputArchive ar(ss);
			ar(deserialized_map);
		}
	} catch (const std::exception &e) {
		LogZoneState("Failed to load entity variables [{}]", e.what());
	}

	return deserialized_map;
}

inline void LoadNPCEntityVariables(NPC *n, const std::string &entity_variables)
{
	if (!RuleB(Zone, StateSaveEntityVariables)) {
		return;
	}

	if (entity_variables.empty()) {
		return;
	}

	for (const auto &[key, value]: GetVariablesDeserialized(entity_variables)) {
		n->SetEntityVariable(key, value);
	}
}

inline void LoadNPCBuffs(NPC *n, const std::string &buffs)
{
	if (!RuleB(Zone, StateSaveBuffs)) {
		return;
	}

	if (buffs.empty()) {
		return;
	}

	if (!Strings::IsValidJson(buffs)) {
		LogZoneState("Invalid JSON data for NPC [{}]", n->GetNPCTypeID());
		return;
	}

	std::vector<Buffs_Struct> valid_buffs;
	try {
		std::istringstream is(buffs);
		{
			cereal::JSONInputArchive archive(is);
			archive(cereal::make_nvp("buffs", valid_buffs));
		}
	}
	catch (const std::exception &e) {
		LogZoneState("Failed to load entity variables for NPC [{}] [{}]", n->GetNPCTypeID(), e.what());
		return;
	}

	n->LoadBuffsFromState(valid_buffs);
}

inline std::vector<uint32_t> GetLootdropIds(const std::vector<ZoneStateSpawnsRepository::ZoneStateSpawns> &spawn_states)
{
	LogInfo("Loading lootdrop ids for zone state spawns");

	std::vector<uint32_t> lootdrop_ids;

	for (auto &s: spawn_states) {
		if (s.loot_data.empty()) {
			continue;
		}

		if (!Strings::IsValidJson(s.loot_data)) {
			continue;
		}

		LootStateData l{};
		try {
			std::stringstream ss;
			{
				ss << s.loot_data;
				cereal::JSONInputArchive ar(ss);
				l.serialize(ar);
			}
		}
		catch (const std::exception &e) {
			LogZoneState("Failed to load loot state data for spawn2 [{}] [{}]", s.id, e.what());
			continue;
		}

		for (auto &e: l.entries) {
			// make sure it isn't already in the list
			if (std::find(lootdrop_ids.begin(), lootdrop_ids.end(), e.lootdrop_id) == lootdrop_ids.end()) {
				lootdrop_ids.push_back(e.lootdrop_id);
			}
		}
	}

	LogInfo("Loaded [{}] lootdrop id(s)", lootdrop_ids.size());

	return lootdrop_ids;
}

inline void LoadNPCStatePreSpawn(Zone *zone, NPC *n, ZoneStateSpawnsRepository::ZoneStateSpawns &s)
{
	LoadNPCEntityVariables(n, s.entity_variables);
}

inline void LoadNPCState(Zone *zone, NPC *n, ZoneStateSpawnsRepository::ZoneStateSpawns &s)
{
	if (s.hp > 0) {
		n->SetHP(s.hp);
	}
	if (s.mana > 0) {
		n->SetMana(s.mana);
	}
	if (s.endurance > 0) {
		n->SetEndurance(s.endurance);
	}

	// if these are zero for some reason, we need to reset the max hp
	if (!s.is_corpse) {
		if (s.hp == 0 || n->GetHP() == 0) {
			n->SetMaxHP();
		}
		if (s.mana == 0 || n->GetMana() == 0) {
			n->RestoreMana();
		}
	}

	if (s.grid) {
		n->AssignWaypoints(s.grid, s.current_waypoint);
	}

	n->SetResumedFromZoneSuspend(false);
	LoadLootStateData(zone, n, s.loot_data);
	n->SetResumedFromZoneSuspend(true);
	LoadNPCBuffs(n, s.buffs);

	if (s.is_corpse) {
		auto decay_time = s.decay_in_seconds * 1000;
		if (decay_time > 0) {
			n->SetQueuedToCorpse();
			entity_list.RestoreCorpse(n, decay_time);
		}
		else {
			n->Depop();
		}
	}

	n->SetPosition(s.x, s.y, s.z);
	n->SetHeading(s.heading);
	n->SetResumedFromZoneSuspend(true);
}

inline std::string GetZoneVariablesSerialized(Zone *z)
{
	std::map<std::string, std::string> variables;

	for (const auto &k: z->GetVariables()) {
		variables[k] = z->GetVariable(k);
	}

	try {
		std::ostringstream os;
		{
			cereal::JSONOutputArchiveSingleLine archive(os);
			archive(variables);
		}
		return os.str();
	}
	catch (const std::exception &e) {
		LogZoneState("Failed to serialize variables for zone [{}]", e.what());
		return "";
	}

	return "";
}

inline void LoadZoneVariables(Zone *z, const std::string &variables)
{
	if (!Strings::IsValidJson(variables)) {
		LogZoneState("Invalid JSON data for zone [{}]", variables);
		return;
	}

	std::map<std::string, std::string> deserialized_map;
	try {
		std::istringstream is(variables);
		{
			cereal::JSONInputArchive archive(is);
			archive(deserialized_map);
		}
	}
	catch (const std::exception &e) {
		LogZoneState("Failed to load zone variables [{}]", e.what());
		return;
	}

	for (const auto &[key, value]: deserialized_map) {
		z->SetVariable(key, value);
	}
}

bool Zone::LoadZoneVariablesState()
{
	auto spawn_states = ZoneStateSpawnsRepository::GetWhere(
		database,
		fmt::format(
			"zone_id = {} AND instance_id = {} AND is_zone = 1 ORDER BY spawn2_id",
			zoneid,
			zone->GetInstanceID()
		)
	);

	if (spawn_states.empty()) {
		return false;
	}

	for (auto &s: spawn_states) {
		if (s.is_zone) {
			LoadZoneVariables(zone, s.entity_variables);
			return true;
		}
	}

	return false;
}

bool Zone::LoadZoneState(
	std::unordered_map<uint32, uint32> spawn_times,
	std::vector<Spawn2DisabledRepository::Spawn2Disabled> disabled_spawns
)
{
	auto spawn_states = ZoneStateSpawnsRepository::GetWhere(
		database,
		fmt::format(
			"zone_id = {} AND instance_id = {} AND is_zone = 0 ORDER BY spawn2_id",
			zoneid,
			zone->GetInstanceID()
		)
	);

	if (spawn_states.empty()) {
		LogInfo("No zone state spawns found for zone [{}] instance [{}]", GetShortName(), zone->GetInstanceID());
		return false;
	}

	LogInfo("Loading zone state spawns for zone [{}] instance [{}] spawns [{}]", GetShortName(), zone->GetInstanceID(), spawn_states.size());

	if (!IsZoneStateValid(spawn_states)) {
		LogZoneState("Invalid zone state data for zone [{}]", GetShortName());
		ClearZoneState(zoneid, zone->GetInstanceID());
		return false;
	}

	std::vector<uint32_t> lootdrop_ids = GetLootdropIds(spawn_states);
	zone->LoadLootDrops(lootdrop_ids);

	// we have to load grids first otherwise setting grid/wp will not work
	zone->initgrids_timer.Trigger();
	zone->Process();

	// load base spawn2 data for spawn locations
	std::vector<std::string> spawn2_ids;
	for (auto &s: spawn_states) {
		if (s.spawn2_id > 0) {
			spawn2_ids.push_back(std::to_string(s.spawn2_id));
		}
	}

	std::vector<Spawn2Repository::Spawn2> spawn2s;
	if (!spawn2_ids.empty()) {
		spawn2s = Spawn2Repository::GetWhere(
			content_db,
			fmt::format(
				"id IN ({})",
				Strings::Join(spawn2_ids, ",")
			)
		);

		LogZoneState("Loaded [{}] spawn2s", spawn2s.size());
	}

	// spawn2
	for (auto &s: spawn_states) {
		if (s.spawngroup_id == 0 || s.is_corpse || s.is_zone) {
			continue;
		}

		uint32 spawn_time_left = 0;
		if (spawn_times.count(s.spawn2_id) != 0) {
			spawn_time_left = spawn_times[s.spawn2_id];
			LogInfo("Spawn2 [{}] Respawn time left [{}]", s.spawn2_id, spawn_time_left);
		}

		// load from spawn2_disabled
		bool spawn_enabled = true;

		// check if spawn is disabled
		for (auto &ds: disabled_spawns) {
			if (ds.spawn2_id == s.spawn2_id) {
				spawn_enabled = !ds.disabled;
			}
		}

		// find spawn 2 by id
		Spawn2Repository::Spawn2 spawn2;
		for (auto &sp: spawn2s) {
			if (sp.id == s.spawn2_id) {
				spawn2 = sp;
				break;
			}
		}

		if (!spawn2.id) {
			LogZoneState("Failed to load spawn2 data for spawn2_id [{}]", s.spawn2_id);
		}

		auto new_spawn = new Spawn2(
			s.spawn2_id,
			s.spawngroup_id,
			spawn2.id > 0 ? spawn2.x : s.x,
			spawn2.id > 0 ? spawn2.y : s.y,
			spawn2.id > 0 ? spawn2.z : s.z,
			spawn2.id > 0 ? spawn2.heading : s.heading,
			s.respawn_time,
			s.variance,
			spawn_time_left,
			s.grid,
			(bool) s.path_when_zone_idle,
			s.condition_id,
			s.condition_min_value,
			(s.enabled && spawn_enabled),
			(EmuAppearance) s.anim
		);

		new_spawn->SetStoredLocation(glm::vec4(s.x, s.y, s.z, s.heading));

		if (spawn_time_left == 0 && s.npc_id > 0) {
			new_spawn->SetResumedNPCID(s.npc_id);
			new_spawn->SetResumedFromZoneSuspend(true);
			new_spawn->SetEntityVariables(GetVariablesDeserialized(s.entity_variables));
		}

		spawn2_list.Insert(new_spawn);
		new_spawn->Process();
		auto n = new_spawn->GetNPC();
		if (n) {
			LoadNPCState(zone, n, s);
		}
	}

	// compare state spawns to spawn2 list, if there are any missing, we need to add them
	// this is to cover the rare case where a spawn2 is created in the database but not in the zone state
	auto zone_spawns = Spawn2Repository::GetWhere(
		content_db, fmt::format(
			"TRUE {} AND zone = '{}' AND (version = {} OR version = -1) ",
			ContentFilterCriteria::apply(),
			zone->GetShortName(),
			zone->GetInstanceVersion()
		)
	);

	for (auto &s: zone_spawns) {
		bool found = false;
		for (auto &ss: spawn_states) {
			if (ss.spawn2_id == 0 || ss.spawngroup_id == 0 || ss.is_corpse || ss.is_zone) {
				continue;
			}
			if (s.id == ss.spawn2_id) {
				found = true;
				break;
			}
		}

		if (!found) {
			bool spawn_enabled = true;

			for (auto &ds: disabled_spawns) {
				if (ds.spawn2_id == s.id) {
					spawn_enabled = !ds.disabled;
				}
			}

			LogZoneState("Missing spawn2 [{}] in zone state, this NPC spawn was newly created", s.id);
			uint32 spawn_time_left = 0;
			if (spawn_times.count(s.id) != 0) {
				spawn_time_left = spawn_times[s.id];
				LogInfo("Spawn2 [{}] Respawn time left [{}]", s.id, spawn_time_left);
			}

			auto new_spawn = new Spawn2(
				s.id,
				s.spawngroupID,
				s.x,
				s.y,
				s.z,
				s.heading,
				s.respawntime,
				s.variance,
				spawn_time_left,
				s.pathgrid,
				(bool) s.path_when_zone_idle,
				s._condition,
				(int16) s.cond_value,
				spawn_enabled,
				(EmuAppearance) s.animation
			);

			new_spawn->SetStoredLocation(glm::vec4(s.x, s.y, s.z, s.heading));
			spawn2_list.Insert(new_spawn);
			new_spawn->Process();
		}
	}

	// dynamic spawns, quest spawns, triggers etc.
	for (auto &s: spawn_states) {
		if (s.spawngroup_id > 0 || s.is_zone) {
			continue;
		}

		auto npc_type = content_db.LoadNPCTypesData(s.npc_id);
		if (!npc_type) {
			LogZoneState("Failed to load NPC type data for npc_id [{}]", s.npc_id);
			continue;
		}

		auto npc = new NPC(
			npc_type,
			nullptr,
			glm::vec4(s.x, s.y, s.z, s.heading),
			GravityBehavior::Water
		);

		npc->SetResumedFromZoneSuspend(true);

		// tag as corpse before we add to entity list to prevent quest triggers
		if (s.is_corpse) {
			npc->SetQueuedToCorpse();
		}

		LoadNPCStatePreSpawn(zone, npc, s);

		entity_list.AddNPC(npc, true, true);

		LoadNPCState(zone, npc, s);
	}

	return !spawn_states.empty();
}

inline void SaveNPCState(NPC *n, ZoneStateSpawnsRepository::ZoneStateSpawns &s)
{
	// entity variables
	std::map<std::string, std::string> variables;

	for (const auto &k: n->GetEntityVariables()) {
		variables[k] = n->GetEntityVariable(k);
	}

	if (!variables.empty()) {
		try {
			std::ostringstream os;
			{
				cereal::JSONOutputArchiveSingleLine archive(os);
				archive(variables);
			}
			s.entity_variables = os.str();
		}
		catch (const std::exception &e) {
			LogZoneState("Failed to serialize entity variables for NPC [{}] [{}]", n->GetNPCTypeID(), e.what());
		}
	}

	// buffs
	auto buffs = n->GetBuffs();
	if (buffs) {
		std::vector<Buffs_Struct> valid_buffs;
		for (int                  index = 0; index < n->GetMaxBuffSlots(); index++) {
			if (buffs[index].spellid != 0 && buffs[index].spellid != 65535) {
				valid_buffs.push_back(buffs[index]);
			}
		}

		if (!valid_buffs.empty()) {
			try {
				std::ostringstream os = std::ostringstream();
				{
					cereal::JSONOutputArchiveSingleLine archive(os);
					archive(cereal::make_nvp("buffs", valid_buffs));
				}
				s.buffs = os.str();
			}
			catch (const std::exception &e) {
				LogZoneState("Failed to serialize buffs for NPC [{}] [{}]", n->GetNPCTypeID(), e.what());
			}
		}
	}

	// rest
	s.npc_id           = n->GetNPCTypeID();
	s.loot_data        = GetLootSerialized(n);
	s.hp               = n->GetHP();
	s.mana             = n->GetMana();
	s.endurance        = n->GetEndurance();
	s.grid             = n->GetGrid();
	s.current_waypoint = n->GetGrid() > 0 ? n->GetCWP() : 0;
	s.x                = n->GetX();
	s.y                = n->GetY();
	s.z                = n->GetZ();
	s.heading          = n->GetHeading();
	s.created_at       = std::time(nullptr);
}

void Zone::SaveZoneState()
{
	// spawns
	std::vector<ZoneStateSpawnsRepository::ZoneStateSpawns> spawns = {};
	LinkedListIterator<Spawn2 *>                            iterator(spawn2_list);
	iterator.Reset();
	int count = 0;
	while (iterator.MoreElements()) {
		Spawn2 *sp = iterator.GetData();
		auto   s   = ZoneStateSpawnsRepository::NewEntity();
		s.zone_id             = GetZoneID();
		s.instance_id         = GetInstanceID();
		s.npc_id              = sp->CurrentNPCID();
		s.spawn2_id           = sp->GetID();
		s.spawngroup_id       = sp->SpawnGroupID();
		s.x                   = sp->GetX();
		s.y                   = sp->GetY();
		s.z                   = sp->GetZ();
		s.heading             = sp->GetHeading();
		s.respawn_time        = sp->RespawnTimer();
		s.variance            = sp->GetVariance();
		s.grid                = sp->GetGrid();
		s.path_when_zone_idle = sp->GetPathWhenZoneIdle() ? 1 : 0;
		s.condition_id        = sp->GetSpawnCondition();
		s.condition_min_value = sp->GetConditionMinValue();
		s.enabled             = sp->Enabled() ? 1 : 0;
		s.anim                = sp->GetAnimation();
		s.created_at          = std::time(nullptr);

		auto n = sp->GetNPC();
		if (n && entity_list.GetNPCByID(n->GetID())) {
			SaveNPCState(n, s);
		}

		spawns.emplace_back(s);
		iterator.Advance();
		count++;
	}

	// npc's that are not in the spawn2 list
	for (auto &n: entity_list.GetNPCList()) {
		// everything below here is dynamically spawned
		bool ignore_npcs =
				 n.second->GetSpawnGroupId() > 0 ||
				 n.second->GetNPCTypeID() < 100 ||
				 n.second->GetNPCTypeID() == 500 || // Trap::CreateHiddenTrigger
				 n.second->IsAura() ||
				 n.second->IsBot() ||
				 n.second->IsMerc() ||
				 n.second->IsTrap() ||
				 n.second->GetSwarmOwner() ||
				 n.second->IsPet();
		if (ignore_npcs) {
			continue;
		}

		auto s = ZoneStateSpawnsRepository::NewEntity();
		s.zone_id     = GetZoneID();
		s.instance_id = GetInstanceID();

		SaveNPCState(n.second, s);

		spawns.emplace_back(s);
	}

	// corpses
	for (auto &n: entity_list.GetCorpseList()) {
		if (!n.second->IsNPCCorpse()) {
			continue;
		}

		auto s = ZoneStateSpawnsRepository::NewEntity();
		s.zone_id          = GetZoneID();
		s.instance_id      = GetInstanceID();
		s.npc_id           = n.second->GetNPCTypeID();
		s.is_corpse        = 1;
		s.x                = n.second->GetX();
		s.y                = n.second->GetY();
		s.z                = n.second->GetZ();
		s.heading          = n.second->GetHeading();
		s.created_at       = std::time(nullptr);
		s.loot_data        = GetLootSerialized(n.second);
		s.decay_in_seconds = (int) (n.second->GetDecayTime() / 1000);

		spawns.emplace_back(s);
	}

	// zone state variables
	if (!GetVariables().empty()) {
		ZoneStateSpawnsRepository::ZoneStateSpawns z{};
		z.zone_id          = GetZoneID();
		z.instance_id      = GetInstanceID();
		z.is_zone          = 1;
		z.entity_variables = GetZoneVariablesSerialized(this);

		spawns.emplace_back(z);
	}

	ZoneStateSpawnsRepository::DeleteWhere(
		database,
		fmt::format(
			"`zone_id` = {} AND `instance_id` = {}",
			GetZoneID(),
			GetInstanceID()
		)
	);

	if (!IsZoneStateValid(spawns)) {
		LogInfo("No valid zone state data to save");
		return;
	}

	if (spawns.empty()) {
		LogInfo("No zone state data to save");
		return;
	}

	ZoneStateSpawnsRepository::InsertMany(database, spawns);

	LogInfo("Saved [{}] zone state spawns", Strings::Commify(spawns.size()));
}

void Zone::ClearZoneState(uint32 zone_id, uint32 instance_id)
{
	ZoneStateSpawnsRepository::DeleteWhere(
		database,
		fmt::format(
			"`zone_id` = {} AND `instance_id` = {}",
			zone_id,
			instance_id
		)
	);
}
