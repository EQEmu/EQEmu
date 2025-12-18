#include "zone/zone_cli.h"
#include "zone/corpse.h"

#include "common/cli/eqemu_command_handler.h"
#include "common/repositories/npc_types_repository.h"
#include "common/repositories/respawn_times_repository.h"

#include "cereal/archives/json.hpp"
#include "cereal/types/map.hpp"

extern Zone* zone;

void RunTest(const std::string& test_name, const std::string& expected, const std::string& actual);
void RunTest(const std::string& test_name, bool expected, bool actual);
void RunTest(const std::string& test_name, int expected, int actual);
void SetupZone(std::string zone_short_name, uint32 instance_id = 0);

inline void ClearState()
{
	ZoneStateSpawnsRepository::DeleteWhere(database, "zone_id = 32 and instance_id = 0");
}

inline std::vector<ZoneStateSpawnsRepository::ZoneStateSpawns> GetStateSpawns()
{
	return ZoneStateSpawnsRepository::GetWhere(database, "zone_id = 32 and instance_id = 0");
}

inline void PrintEntityCounts()
{
	std::cout << " NPC count: " << entity_list.GetNPCList().size() << std::endl;
	std::cout << " Corpse count: " << entity_list.GetCorpseList().size() << std::endl;
}

inline void PrintZoneNpcs()
{
	int       npc_count = 0;
	for (auto &npc: entity_list.GetNPCList()) {
		std::cout << npc.second->GetNPCTypeID() << " " << npc.second->GetCleanName() << std::endl;
		npc_count++;
	}

	std::cout << "Total spawned NPCs: " << npc_count << std::endl;
}

inline void SetupStateZone()
{
	SetupZone("soldungb");
	zone->Process();
	// depop the zone controller
	auto controller = entity_list.GetNPCByNPCTypeID(ZONE_CONTROLLER_NPC_ID);
	if (controller != nullptr) {
		controller->Depop();
	}

	entity_list.MobProcess(); // process the depop
}

inline int GetStateSpawnSpawn2Count()
{
	int       count = 0;
	for (auto &e: GetStateSpawns()) {
		if (e.spawn2_id > 0) {
			count++;
		}
	}
	return count;
}

inline uint32_t SeedLootTable()
{
	const std::string table_name = "zone_state_test";

	auto entries = LoottableRepository::GetWhere(database, fmt::format("name = '{}'", table_name));
	if (!entries.empty()) {
		zone->LoadLootTable(entries[0].id);
		return entries[0].id;
	}

	// seed the loot table
	auto loot_table = LoottableRepository::NewEntity();
	loot_table.name = table_name;
	auto inserted = LoottableRepository::InsertOne(database, loot_table);

	auto loot_drop = LootdropRepository::NewEntity();
	loot_drop.name = table_name;
	auto inserted_loot_drop = LootdropRepository::InsertOne(database, loot_drop);

	auto loot_table_entries = LoottableEntriesRepository::NewEntity();
	loot_table_entries.lootdrop_id  = inserted_loot_drop.id;
	loot_table_entries.loottable_id = inserted.id;
	LoottableEntriesRepository::InsertOne(database, loot_table_entries);

	auto loot_drop_entries = LootdropEntriesRepository::NewEntity();
	loot_drop_entries.lootdrop_id = inserted_loot_drop.id;
	loot_drop_entries.item_id     = 11621; // cloak of flames
	loot_drop_entries.chance      = 100;
	LootdropEntriesRepository::InsertOne(database, loot_drop_entries);

	zone->LoadLootTable(inserted.id);

	return inserted.id;
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

// MatchState compares the NPC to the state
// it should print what doesn't match when it returns false
inline bool MatchState(NPC *n, const ZoneStateSpawnsRepository::ZoneStateSpawns &state)
{
	if (n->GetNPCTypeID() != state.npc_id) {
		std::cout << "NPC ID mismatch: " << n->GetNPCTypeID() << " != " << state.npc_id << std::endl;
		return false;
	}

	if (n->GetX() != state.x) {
		std::cout << "X mismatch: " << n->GetX() << " != " << state.x << std::endl;
		return false;
	}

	if (n->GetY() != state.y) {
		std::cout << "Y mismatch: " << n->GetY() << " != " << state.y << std::endl;
		return false;
	}

	if (n->GetZ() != state.z) {
		std::cout << "Z mismatch: " << n->GetZ() << " != " << state.z << std::endl;
		return false;
	}

	if (n->GetHeading() != state.heading) {
		std::cout << "Heading mismatch: " << n->GetHeading() << " != " << state.heading << std::endl;
		return false;
	}

	if (n->GetHP() != state.hp) {
		std::cout << "HP mismatch: " << n->GetHP() << " != " << state.hp << std::endl;
		return false;
	}

	if (n->GetMana() != state.mana) {
		std::cout << "Mana mismatch: " << n->GetMana() << " != " << state.mana << std::endl;
		return false;
	}

	if (n->GetEndurance() != state.endurance) {
		std::cout << "Endurance mismatch: " << n->GetEndurance() << " != " << state.endurance << std::endl;
		return false;
	}

	auto entity_variables = GetVariablesDeserialized(state.entity_variables);

	for (const auto &[key, value]: entity_variables) {
		if (n->GetEntityVariable(key) != value) {
			std::cout << "Variable mismatch: " << key << " " << n->GetEntityVariable(key) << " != " << value
					  << std::endl;
			return false;
		}
	}

	return true;
}

// MatchFreshlySpawnedState compares the NPC to the state
// it should print what doesn't match when it returns false
inline bool MatchFreshlySpawnedState(NPC *n, const ZoneStateSpawnsRepository::ZoneStateSpawns &state)
{
	auto npc_type = content_db.LoadNPCTypesData(n->GetNPCTypeID());
	if (!npc_type) {
		RunTest("NPC type does not exist", false, true);
	}

	if (n->GetX() != state.x) {
		std::cout << "X mismatch: " << n->GetX() << " != " << state.x << std::endl;
		return false;
	}

	if (n->GetY() != state.y) {
		std::cout << "Y mismatch: " << n->GetY() << " != " << state.y << std::endl;
		return false;
	}

//	if (n->GetZ() != state.z) {
//		std::cout << "Z mismatch: " << n->GetZ() << " != " << state.z << std::endl;
//		return false;
//	}

	if (n->GetHeading() != state.heading) {
		std::cout << "Heading mismatch: " << n->GetHeading() << " != " << state.heading << std::endl;
		return false;
	}

	if (n->GetHP() != npc_type->max_hp) {
		std::cout << "HP mismatch: " << n->GetHP() << " != " << npc_type->max_hp << std::endl;
		return false;
	}

	if (n->GetLoottableID() != npc_type->loottable_id) {
		std::cout << "Loot table mismatch: " << n->GetLoottableID() << " != " << npc_type->loottable_id << std::endl;
		return false;
	}

	// not loaded from database
//	if (n->GetMana() != npc_type->Mana) {
//		std::cout << "Mana mismatch: " << n->GetMana() << " != " << npc_type->Mana << std::endl;
//		return false;
//	}

	// endurance not saved

	auto entity_variables = GetVariablesDeserialized(state.entity_variables);

	for (const auto &[key, value]: entity_variables) {
		if (n->GetEntityVariable(key) != value) {
			std::cout << "Variable mismatch: " << key << " " << n->GetEntityVariable(key) << " != " << value
					  << std::endl;
			return false;
		}
	}

	return true;
}

inline bool MatchCorpseState(Corpse *c, const ZoneStateSpawnsRepository::ZoneStateSpawns &state)
{
	if (c->GetNPCTypeID() != state.npc_id) {
		std::cout << "Corpse NPC ID mismatch: " << c->GetNPCTypeID() << " != " << state.npc_id << std::endl;
		return false;
	}

	if (c->GetX() != state.x) {
		std::cout << "Corpse X mismatch: " << c->GetX() << " != " << state.x << std::endl;
		return false;
	}

	if (c->GetY() != state.y) {
		std::cout << "Corpse Y mismatch: " << c->GetY() << " != " << state.y << std::endl;
		return false;
	}

//	if (c->GetZ() != state.z) {
//		std::cout << "Corpse Z mismatch: " << c->GetZ() << " != " << state.z << std::endl;
//		return false;
//	}

	if (c->GetHeading() != state.heading) {
		std::cout << "Corpse Heading mismatch: " << c->GetHeading() << " != " << state.heading << std::endl;
		return false;
	}

	if (c->GetDecayTime() != state.decay_in_seconds * 1000) {
		std::cout << "Corpse Decay Time mismatch: " << c->GetDecayTime() << " != " << state.decay_in_seconds * 1000
				  << std::endl;
		return false;
	}

	auto entity_variables = GetVariablesDeserialized(state.entity_variables);

	for (const auto &[key, value]: entity_variables) {
		if (c->GetEntityVariable(key) != value) {
			std::cout << "Corpse Variable mismatch: " << key << " " << c->GetEntityVariable(key) << " != " << value
					  << std::endl;
			return false;
		}
	}

	return true;
}

inline void TestSpawns()
{
	zone->Shutdown();
	ClearState();
	SetupStateZone();
	zone->Repop(true);
	entity_list.MobProcess();

//	PrintZoneNpcs();

	RunTest("Spawns > Ensure no state spawns exist before shutdown", 0, (int) GetStateSpawns().size());

	zone->Shutdown();

	auto entries = GetStateSpawns().size();
	RunTest(fmt::format("Spawns > State exists after shutdown, entries ({})", entries), true, entries > 0);

	SetupStateZone();

	entries = GetStateSpawns().size();
	RunTest(fmt::format("Spawns > State exists after bootup, entries ({})", entries), true, entries > 0);

	zone->Shutdown();
	SetupStateZone();

	entries = GetStateSpawns().size();
	RunTest(
		fmt::format("Spawns > State is the same after shutdown/bootup (2nd time), entries ({})", entries),
		true,
		entries > 0
	);

	// need to compare the state spawns to the actual spawns
	bool all_exist = true;

	for (auto &state_spawn: GetStateSpawns()) {
		auto npc = entity_list.GetNPCByNPCTypeID(state_spawn.npc_id);
		if (!npc) {
			all_exist = false;
			break;
		}
	}

	RunTest("Spawns > All state spawns (Spawn2) exist in entity list", true, all_exist);
	RunTest(
		fmt::format("Spawns > All state spawns (Spawn2) exist in entity list, count [{}]", GetStateSpawns().size()),
		true,
		GetStateSpawns().size() == entity_list.GetNPCList().size()
	);
	RunTest(
		fmt::format(
			"Spawns > Same count of state spawns and entity list | state spawns [{}] entity list [{}]",
			GetStateSpawns().size(),
			entity_list.GetNPCList().size()),
		true,
		GetStateSpawns().size() == entity_list.GetNPCList().size()
	);

	// kill all NPC's
	std::vector<NPC *> npcs_to_kill;

	// Collect NPCs first
	for (const auto &e: entity_list.GetNPCList()) {
		if (e.second) {
			npcs_to_kill.push_back(e.second);
		}
	}

	// Now safely process them
	for (auto *npc: npcs_to_kill) {
		npc->SetQueuedToCorpse();
		npc->Death(npc, npc->GetHP() + 1, SPELL_UNKNOWN, EQ::skills::SkillHandtoHand);
	}

	bool condition = (int) entity_list.GetNPCList().size() == 0 && (int) entity_list.GetCorpseList().size() == entries;
	RunTest(
		fmt::format("Spawns > All NPC's killed (0 NPCs) ([{}] Corpses)", entries),
		true,
		condition
	);

	std::vector<uint32_t> spawn2_ids = {};

	for (auto &e: GetStateSpawns()) {
		if (e.spawn2_id > 0) {
			spawn2_ids.push_back(e.spawn2_id);
		}
	}

	auto times = RespawnTimesRepository::GetWhere(
		database,
		fmt::format("id IN ({})", Strings::Join(spawn2_ids, ","))
	);

	for (auto &e: times) {
		e.duration = 1;
		e.start    = std::time(nullptr) + 1;
	}
	RespawnTimesRepository::ReplaceMany(database, times);

	zone->Shutdown();
	SetupStateZone();

	condition = (int) entity_list.GetNPCList().size() == 0 && (int) entity_list.GetCorpseList().size() == entries;
	if (!condition) {
		PrintEntityCounts();
	}
	RunTest(
		fmt::format("Spawns > After restore (0 NPCs) ([{}] Corpses)", entries),
		true,
		condition
	);

	for (auto &e: entity_list.GetCorpseList()) {
		auto c = e.second;

		for (auto &s: GetStateSpawns()) {
			bool is_same_corpse =
					 c->GetNPCTypeID() == s.npc_id &&
					 c->GetX() == s.x &&
					 c->GetY() == s.y &&
					 c->GetHeading() == s.heading;

			if (is_same_corpse) {
				if (!MatchCorpseState(c, s)) {
					RunTest("Spawns > Corpse state matches state", true, false);
				}
			}
		}
	}

	Timer::RollForward(5);

	zone->Process();

	condition = (int) entity_list.GetNPCList().size() == entries && (int) entity_list.GetCorpseList().size() == entries;
	if (!condition) {
		PrintEntityCounts();
	}
	RunTest(
		fmt::format("Spawns > After respawn ([{}] NPCs) ([{}] Corpses)", entries, entries),
		true,
		condition
	);

	for (auto &c: entity_list.GetCorpseList()) {
		c.second->DepopNPCCorpse();
	}

	entity_list.CorpseProcess();

	condition = (int) entity_list.GetNPCList().size() == entries && (int) entity_list.GetCorpseList().size() == 0;
	if (!condition) {
		PrintEntityCounts();
	}
	RunTest(
		fmt::format("Spawns > After respawn ([{}] NPCs) (0 Corpses)", entries),
		true,
		condition
	);

	// lets set NPC's up with a predictable loottable for testing
	uint32_t loottable_id = SeedLootTable();

	for (auto &e: entity_list.GetNPCList()) {
		auto n = e.second;
		n->ClearLootItems();
		n->SetResumedFromZoneSuspend(false);
		n->AddLootTable(loottable_id);
		n->SetResumedFromZoneSuspend(true);
	}

	RespawnTimesRepository::DeleteWhere(database, fmt::format("id IN ({})", Strings::Join(spawn2_ids, ",")));

	zone->Shutdown();
	SetupStateZone();

	npcs_to_kill = {};

	// kill only 10 NPCs
	int i = 0;

	for (const auto &e: entity_list.GetNPCList()) {
		if (e.second) {
			npcs_to_kill.push_back(e.second);
			i++;
		}
		if (i == 10) {
			break;
		}
	}

	for (auto *npc: npcs_to_kill) {
		if (!npc) {
			continue;
		}
		npc->SetQueuedToCorpse();
		npc->Death(npc, npc->GetHP() + 1, SPELL_UNKNOWN, EQ::skills::SkillHandtoHand);
	}

	condition = (int) entity_list.GetNPCList().size() == (entries - 10) && (int) entity_list.GetCorpseList().size() == 10;
	if (!condition) {
		PrintEntityCounts();
	}
	RunTest(
		fmt::format("Spawns > Kill 10 NPC's before save/restore ([{}] NPCs) (10 Corpses)", (entries - 10)),
		true,
		condition
	);

	zone->Shutdown();
	SetupStateZone();

	condition = (int) entity_list.GetNPCList().size() == (entries - 10) && (int) entity_list.GetCorpseList().size() == 10;
	if (!condition) {
		PrintEntityCounts();
	}
	RunTest(
		fmt::format("Spawns > After restore ([{}] NPCs) (10 Corpses)", (entries - 10)),
		true,
		condition
	);

	// validate that all corpses and npc's have cloak of flames
	bool      test_failed = false;
	for (auto &e: entity_list.GetCorpseList()) {
		auto corpse = e.second;
		if (!corpse) {
			continue;
		}

		bool has_item = corpse->HasItem(11621);
		if (!has_item || corpse->CountItem(11621) != 1) {
			std::cout << "Corpse does not have item" << std::endl;
			std::cout << " -- CountItem(11621) " << corpse->CountItem(11621) << std::endl;
			std::cout << " -- GetCleanName " << corpse->GetCleanName() << std::endl;
			RunTest(
				"Spawns > After restore > Corpse has item (cloak of flames) item count (1) (no dupes)",
				true,
				has_item
			);
		}
	}

	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		if (!npc) {
			continue;
		}

		bool has_item = npc->HasItem(11621);
		if (!has_item && npc->CountItem(11621) == 1) {
			std::cout << "NPC does not have item" << std::endl;
			std::cout << " -- CountItem(11621) " << npc->CountItem(11621) << std::endl;
			std::cout << " -- GetCleanName " << npc->GetCleanName() << std::endl;
			RunTest(
				"Spawns > After restore > NPC has item (cloak of flames) item count (1) (no dupes)",
				true,
				has_item
			);
		}
	}

	RunTest(
		"Spawns > After restore > All npcs/corpses have item (cloak of flames) item count (1) and no dupes",
		true,
		true
	);

	// test to make sure the entity variables match what was on state
	int npcs_matching_state = 0;

	for (auto &e: entity_list.GetNPCList()) {
		auto      npc = e.second;
		for (auto &s: GetStateSpawns()) {
			if (npc->GetSpawnGroupId() == s.spawngroup_id && npc->GetSpawn()->GetID() == s.spawn2_id) {
				if (!MatchState(npc, s)) {
					RunTest("Spawns > NPC state matches state", true, false);
				}
				npcs_matching_state++;
			}
		}
	}

	RunTest(
		fmt::format("Spawns > After restore > NPC(s) matching state | matching ({})", npcs_matching_state),
		false,
		false
	);

	// test to make sure the entity variables match what was on state
	int corpses_matching_state = 0;

	for (auto &e: entity_list.GetCorpseList()) {
		auto c = e.second;

		for (auto &s: GetStateSpawns()) {
			bool is_same_corpse =
					 c->GetNPCTypeID() == s.npc_id &&
					 c->GetX() == s.x &&
					 c->GetY() == s.y &&
					 c->GetHeading() == s.heading;

			if (is_same_corpse) {
				if (!MatchCorpseState(c, s)) {
					RunTest("Spawns > Corpse state matches state", true, false);
				}
				corpses_matching_state++;
			}
		}
	}

	RunTest(
		fmt::format("Spawns > After restore > Corpse(s) matching state | matching ({})", corpses_matching_state),
		false,
		false
	);

	int max_respawn = 0;
	const auto& l = RespawnTimesRepository::All(database);
	for (const auto& e : l) {
		if (e.duration > max_respawn) {
			max_respawn = e.duration;
		}
	}

	entity_list.MobProcess();

	zone->Process();
	zone->ClearSpawnTimers();
	entity_list.MobProcess();

	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		if (npc->GetSpawnGroupId() == 0) {
			continue;
		}

		npc->SetEntityVariable("previously_spawned", "true");
	}

	Timer::RollForward(max_respawn); // longest respawn time in zone
	zone->Process();
	entity_list.MobProcess(); // processing depops

	condition = (int) entity_list.GetNPCList().size() == entries && (int) entity_list.GetCorpseList().size() == 10;
	if (!condition) {
		PrintEntityCounts();
		PrintZoneNpcs();
	}
	RunTest(
		fmt::format("Spawns > After respawn, ensure we have expected entity counts ([{}] NPCs) (10 Corpses)", entries),
		true,
		condition
	);

	entity_list.MobProcess(); // processing depops

	npcs_matching_state = 0;
	int dropped_items = 0;

	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;

		if (npc->GetEntityVariable("previously_spawned") != "true") {
			for (auto &s: GetStateSpawns()) {
				if (npc->GetSpawnGroupId() == s.spawngroup_id && npc->GetSpawn()->GetID() == s.spawn2_id &&
					s.npc_id == 0) {
					if (!MatchFreshlySpawnedState(npc, s)) {
						RunTest("Spawns > NPC state matches state", true, false);
					}

					dropped_items += npc->GetLootList().size();
					npcs_matching_state++;
				}
			}
		}
	}

	RunTest(
		fmt::format(
			"Spawns > After restore, after respawn (10 killed) > ensure NPC(s) match state | matching ({})",
			npcs_matching_state
		),
		npcs_matching_state == 10,
		true
	);

	RunTest(
		fmt::format(
			"Spawns > After restore, after respawn (10 killed) ensure NPC's have loot assigned | dropped items ({})",
			dropped_items
		),
		dropped_items > 0,
		true
	);
}

inline void TestZoneVariables()
{
	std::vector<std::pair<std::string, std::string>> test_variables = {
		{"test_variable",  "test_value"},
		{"test_variable2", "test_value2"}
	};

	// Set variables
	for (const auto &[key, value]: test_variables) {
		zone->SetVariable(key, value);
		RunTest("Zone variables > (" + key + ") set", value, zone->GetVariable(key));
	}

	// Simulate shutdown and restart twice
	for (int i = 1; i <= 2; ++i) {
		zone->Shutdown();
		SetupStateZone();

		for (const auto &[key, value]: test_variables) {
			RunTest(
				"Zone variables > (" + key + ") persists after shutdown/bootup (" + std::to_string(i) + ")",
				value,
				zone->GetVariable(key)
			);
		}
	}

	// Delete one variable
	zone->DeleteVariable("test_variable");

	RunTest("Zone variables > (test_variable) delete is empty", "", zone->GetVariable("test_variable"));
	RunTest(
		"Zone variables > (test_variable2) delete second one still exists",
		"test_value2",
		zone->GetVariable("test_variable2")
	);

	// Final shutdown and restart check
	zone->Shutdown();
	SetupStateZone();

	for (const auto &[key, value]: test_variables) {
		std::string expected_value = (key == "test_variable") ? "" : value;
		RunTest("Zone variables > (" + key + ") after shutdown/bootup", expected_value, zone->GetVariable(key));
	}
}

inline void TestHpManaEnd()
{
	std::vector<uint32_t> ids = {};

	for (auto &npc: entity_list.GetNPCList()) {
		ids.push_back(npc.second->GetNPCTypeID());
	}

	auto npc_types = NpcTypesRepository::GetWhere(database, fmt::format(" id IN ({})", Strings::Join(ids, ",")));

	// validate that hp / mana / end equal that of what's on the npc types row for all rows
	// dont run tests in the loop, just collect the data
	std::vector<std::pair<uint32_t, bool>> hp_mismatch   = {};
	std::vector<std::pair<uint32_t, bool>> mana_mismatch = {};

	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;

		for (auto &npc_type: npc_types) {
			if (npc->GetNPCTypeID() != npc_type.id) {
				continue;
			}

			// we need to make sure the values are not 0 because of auto scaling
			if (npc->GetHP() != npc_type.hp && npc_type.hp > 0) {
				hp_mismatch.emplace_back(npc->GetNPCTypeID(), true);
			}
			if (npc->GetMana() != npc_type.mana && npc_type.mana > 0) {
				mana_mismatch.emplace_back(npc->GetNPCTypeID(), true);
			}
		}
	}

	RunTest(
		"HP/Mana/End Save/Restore > Ensure default HP state matches data in npc_types row",
		0,
		(int) hp_mismatch.size()
	);

	RunTest(
		"HP/Mana/End Save/Restore > Ensure default Mana state matches data in npc_types row",
		0,
		(int) mana_mismatch.size()
	);

	// do damage to NPC's and make sure they restore to their original values
	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		npc->SetHP(1);
		npc->SetMana(1);
	}

	zone->Shutdown();
	SetupStateZone();

	hp_mismatch.clear();
	mana_mismatch.clear();

	// compare state values versus what's on the entity list NPC object
	for (auto &e: entity_list.GetNPCList()) {
		auto      npc = e.second;
		for (auto &state: GetStateSpawns()) {
			if (npc->GetNPCTypeID() != state.npc_id) {
				continue;
			}

			// we need to make sure the values are not 0 because of auto scaling
			if (npc->GetHP() != state.hp && state.hp > 0) {
				hp_mismatch.emplace_back(npc->GetNPCTypeID(), true);
			}
			if (npc->GetMana() != state.mana && state.mana > 0) {
				mana_mismatch.emplace_back(npc->GetNPCTypeID(), true);
			}
		}
	}

	RunTest(
		"HP/Mana/End Save/Restore > Ensure restored HP state matches data on the NPC object",
		0,
		(int) hp_mismatch.size()
	);
	RunTest(
		"HP/Mana/End Save/Restore > Ensure restored Mana state matches data on the NPC object",
		0,
		(int) mana_mismatch.size()
	);
}

inline void TestBuffs()
{
	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		if (npc->GetNPCTypeID() == 0) {
			continue;
		}

		npc->CastSpell(6824, npc->GetID(), (EQ::spells::CastingSlot) 0, 0, 0);
	}

	zone->Shutdown();
	SetupStateZone();

	// Check buffs
	bool missing_buffs = false;

	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		if (npc->GetNPCTypeID() == 0) {
			continue;
		}

		if (!npc->FindBuff(6824)) {
			missing_buffs = true;
			break;
		}
	}

	RunTest("Buffs > Persist after shutdown/bootup", false, missing_buffs);
}

inline void TestZLocationDrift()
{
	zone->Shutdown();
	ClearState();
	SetupStateZone();

	auto b = GetStateSpawns();

	for (int i = 0; i < 10; ++i) {
		zone->Shutdown();
		SetupStateZone();
	}

	auto a = GetStateSpawns();

	// compare entries_before x/y/z to entries_after x/y/z
	bool locations_different = false;

	for (size_t i = 0; i < b.size(); ++i) {
		if (b[i].x != a[i].x || b[i].y != a[i].y || b[i].z != a[i].z) {
			locations_different = true;

			std::cout << "Location drift detected for NPC ID: " << b[i].npc_id << std::endl;
			std::cout << "Location drift detected for NPC ID: " << a[i].npc_id << std::endl;
			std::cout << "Before - X: " << b[i].x << ", Y: " << b[i].y << ", Z: " << b[i].z << std::endl;
			std::cout << "After  - X: " << a[i].x << ", Y: " << a[i].y << ", Z: " << a[i].z << std::endl;
			break;
		}
	}

	RunTest(
		"Z Location Drift > Ensure Z location does not drift after multiple shutdowns/bootups",
		false,
		locations_different
	);
}

inline void TestLocationChange()
{
	// Test that state spawns are where we moved them to if we moved them, move them slightly in a predictable way so
	// we can test for it after restore
	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		if (!npc) {
			continue;
		}

		// set all to -870	-1394	106.58 (nagafen area)
		npc->SetPosition(-870, -1394, 106);
	}

	zone->Shutdown();
	SetupStateZone();

	bool all_moved = true;

	for (auto &e: entity_list.GetNPCList()) {
		for (auto &state: GetStateSpawns()) {
			if (e.second->GetNPCTypeID() != state.npc_id) {
				continue;
			}

			auto n = e.second;
			if (n->GetSpawnGroupId() != state.spawngroup_id && n->GetSpawn()->GetID() != state.spawn2_id) {
				continue;
			}

			// z gets auto adjusted to the ground, so we dont need to check it
			if (n->GetX() != state.x || n->GetY() != state.y) {
				std::cout << "NPC ID: " << n->GetNPCTypeID() << " X: " << n->GetX() << " Y: " << n->GetY() << std::endl;
				std::cout << "State ID " << state.npc_id << " X: " << state.x << " Y: " << state.y << std::endl;
				std::cout << "-----------------------------------\n";
				all_moved = false;
				break;
			}
			else {
//				std::cout << "NPC ID: " << e.second->GetNPCTypeID() << " X: " << e.second->GetX() << " Y: " << e.second->GetY() << std::endl;
//				std::cout << "State ID " << state.npc_id << " X: " << state.x << " Y: " << state.y << std::endl;
//				std::cout << "-----------------------------------\n";
			}
		}
	}

	RunTest("Location > State spawns are where we moved them to after restore", true, all_moved);
}

inline void TestEntityVariables()
{
	std::map<std::string, std::string> test_entity_variables = {
		{"test_entity_variable",  "test_entity_value"},
		{"test_entity_variable2", "test_entity_value2"}
	};

	// Set entity variables
	for (const auto &[key, value]: test_entity_variables) {
		for (auto &e: entity_list.GetNPCList()) {
			e.second->SetEntityVariable(key, value);
		}
	}

	zone->Shutdown();
	SetupStateZone();

	// Check entity variables
	bool missing_entity_variables = false;
	for (const auto &[key, value]: test_entity_variables) {
		for (auto &e: entity_list.GetNPCList()) {
			if (e.second->GetEntityVariable(key) != value) {
				missing_entity_variables = true;
				break;
			}
		}
	}

	RunTest("Entity Variables > Persist after shutdown/bootup", false, missing_entity_variables);
}

inline void TestLoot()
{
	uint32_t table_id = SeedLootTable();

	for (auto &e: entity_list.GetNPCList()) {
		e.second->ClearLootItems();
		e.second->SetResumedFromZoneSuspend(false);
		e.second->AddLootTable(table_id);
		e.second->SetResumedFromZoneSuspend(true);
	}

	bool missing_loot = false;

	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		if (npc->GetNPCTypeID() == 0) {
			continue;
		}

		// cloak of flames
		if (npc->CountItem(11621) == 0) {
			missing_loot = true;
			break;
		}
	}

	RunTest("Loot > Cloak of Flames added to all NPC's via Loottable before shutdown", false, missing_loot);

	zone->Shutdown();
	SetupStateZone();

	missing_loot = false;
	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		if (npc->GetNPCTypeID() == 0) {
			continue;
		}

		// cloak of flames
		if (npc->CountItem(11621) == 0) {
			missing_loot = true;
			break;
		}
	}

	RunTest("Loot > Cloak of Flames added to all NPC's via Loottable after shutdown/bootup", false, missing_loot);

	// make sure no duplicates are added
	bool duplicates = false;

	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		if (npc->GetNPCTypeID() == 0) {
			continue;
		}

		if (npc->CountItem(11621) > 1) {
			duplicates = true;
			break;
		}
	}

	RunTest("Loot > No duplicates added when adding item to NPC", false, duplicates);

	// kill all NPC's
	std::vector<NPC *> npcs_to_kill;

	// Collect NPCs first
	for (const auto &e: entity_list.GetNPCList()) {
		if (e.second) {
			npcs_to_kill.push_back(e.second);
		}
	}

	// Now safely process them
	for (auto *npc: npcs_to_kill) {
		npc->SetQueuedToCorpse();
		npc->Death(npc, npc->GetHP() + 1, SPELL_UNKNOWN, EQ::skills::SkillHandtoHand);
	}

	// make sure all of the corpses have "Cloak of Flames"
	bool missing_loot_corpse = false;

	for (auto &e: entity_list.GetCorpseList()) {
		auto corpse = e.second;
		if (corpse->GetNPCTypeID() == 0) {
			continue;
		}

		if (corpse->CountItem(11621) == 0) {
			missing_loot_corpse = true;
			break;
		}
	}

	RunTest(
		"Loot > Cloak of Flames added to all Corpse's via Loottable before shutdown/bootup",
		false,
		missing_loot_corpse
	);

	zone->Shutdown();
	SetupStateZone();

	missing_loot_corpse = false;
	for (auto &e: entity_list.GetCorpseList()) {
		auto corpse = e.second;
		if (corpse->GetNPCTypeID() == 0) {
			continue;
		}

		if (corpse->CountItem(11621) == 0) {
			missing_loot_corpse = true;
			break;
		}
	}

	RunTest(
		"Loot > Cloak of Flames added to all Corpse's via Loottable after shutdown/bootup",
		false,
		missing_loot_corpse
	);

	// make sure no duplicates are added
	bool duplicates_corpse = false;

	for (auto &e: entity_list.GetCorpseList()) {
		auto corpse = e.second;
		if (corpse->GetNPCTypeID() == 0) {
			continue;
		}

		if (corpse->CountItem(11621) > 1) {
			duplicates_corpse = true;
			break;
		}
	}

	RunTest("Loot > No duplicates added when adding item to Corpse", false, duplicates_corpse);
}

void ZoneCLI::TestZoneState(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	if (cmd[{"-h", "--help"}]) {
		return;
	}

	ClearState(); // clean slate
	SetupStateZone();
	zone->Repop(true);

	std::cout << "===========================================\n";
	std::cout << "⚙\uFE0F> Running Zone State Tests... (soldungb)\n";
	std::cout << "===========================================\n\n";

	TestZoneVariables();
	TestHpManaEnd();
	TestBuffs();
	TestLocationChange();
	TestEntityVariables();
	TestLoot();
	TestSpawns();
	TestZLocationDrift();

	std::cout << "\n===========================================\n";
	std::cout << "✅ All Zone State Tests Completed!\n";
	std::cout << "===========================================\n";
}
