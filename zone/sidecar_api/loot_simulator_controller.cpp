#include "sidecar_api.h"

#include "common/json/json.hpp"
#include "zone/zone.h"

extern Zone *zone;

void SidecarApi::LootSimulatorController(const httplib::Request &req, httplib::Response &res)
{
	int  loottable_id = req.has_param("loottable_id") ? std::stoi(req.get_param_value("loottable_id")) : 4027;
	int  npc_id       = req.has_param("npc_id") ? std::stoi(req.get_param_value("npc_id")) : 32040; // lord nagafen
	auto iterations   = 100;
	auto log_enabled  = false;

	EQEmuLogSys::Instance()->log_settings[Logs::Loot].log_to_console = 0;

	nlohmann::json j;

	auto npc_type = content_db.LoadNPCTypesData(npc_id);
	if (npc_type) {
		auto       npc = new NPC(
			npc_type,
			nullptr,
			glm::vec4(0, 0, 0, 0),
			GravityBehavior::Water
		);
		BenchTimer benchmark;

		// depop the previous one
		for (auto &n: entity_list.GetNPCList()) {
			if (n.second->GetNPCTypeID() == npc_id) {
				LogInfo("found npc id [{}]", npc_id);
				n.second->Depop(false);
			}
		}

		entity_list.Process();
		entity_list.MobProcess();
		npc->SetRecordLootStats(true);

		for (int i = 0; i < iterations; i++) {
			npc->AddLootTable(loottable_id);

			for (auto &id: zone->GetGlobalLootTables(npc)) {
				npc->AddLootTable(id);
			}
		}

		entity_list.AddNPC(npc);

		j["data"]["loottable_id"]       = loottable_id;
		j["data"]["npc_id"]             = npc_id;
		j["data"]["npc_name"]           = npc->GetCleanName();
		j["data"]["rolled_items_count"] = npc->GetRolledItems().size();
		j["data"]["iterations"]         = iterations;

		// npc level loot table
		auto loot_table = zone->GetLootTable(loottable_id);
		if (!loot_table) {
			res.status = 400;
			j["error"] = fmt::format("Loot table not found [{}]", loottable_id);
			res.set_content(j.dump(), "application/json");
			return;
		}

		auto le = zone->GetLootTableEntries(loottable_id);

		// translate above for loop using le
		for (auto &e: le) {
			auto loot_drop = zone->GetLootdrop(e.lootdrop_id);
			if (!loot_drop.id) {
				continue;
			}

			LogLootDetail(
				"# Lootdrop ID [{}] drop_limit [{}] min_drop [{}] mult [{}] probability [{}]",
				e.lootdrop_id,
				e.droplimit,
				e.mindrop,
				e.multiplier,
				e.probability
			);

			nlohmann::json jle;
			jle["lootdrop_id"] = e.lootdrop_id;
			jle["droplimit"]   = e.droplimit;
			jle["mindrop"]     = e.mindrop;
			jle["multiplier"]  = e.multiplier;
			jle["probability"] = e.probability;

			auto loot_drop_entries = zone->GetLootdropEntries(e.lootdrop_id);
			int  slot              = 0;

			for (auto &f: loot_drop_entries) {
				int                rolled_count = npc->GetRolledItemCount(f.item_id);
				const EQ::ItemData *item        = database.GetItem(f.item_id);

				auto rolled_percentage = (float) ((float) ((float) rolled_count / (float) iterations) * 100);

				nlohmann::json drop;
				drop["slot"]                       = slot;
				drop["item_id"]                    = f.item_id;
				drop["item_name"]                  = item->Name;
				drop["chance"]                     = fmt::format("{:.2f}", f.chance);
				drop["simulate_rolled_count"]      = rolled_count;
				drop["simulate_rolled_percentage"] = fmt::format("{:.2f}", rolled_percentage);
				jle["drops"].push_back(drop);
				slot++;
			}

			j["lootdrops"].push_back(jle);
		}

		// global loot
		for (auto &id: zone->GetGlobalLootTables(npc)) {
			loot_table = zone->GetLootTable(id);
			if (!loot_table) {
				LogInfo("Global Loot table not found [{}]", id);
				continue;
			}

			le = zone->GetLootTableEntries(id);

			// translate above for loop using le
			for (auto &e: le) {
				auto loot_drop = zone->GetLootdrop(e.lootdrop_id);
				if (!loot_drop.id) {
					continue;
				}

				LogLootDetail(
					"# Lootdrop ID [{}] drop_limit [{}] min_drop [{}] mult [{}] probability [{}]",
					e.lootdrop_id,
					e.droplimit,
					e.mindrop,
					e.multiplier,
					e.probability
				);

				nlohmann::json jle;
				jle["lootdrop_id"] = e.lootdrop_id;
				jle["droplimit"]   = e.droplimit;
				jle["mindrop"]     = e.mindrop;
				jle["multiplier"]  = e.multiplier;
				jle["probability"] = e.probability;

				auto loot_drop_entries = zone->GetLootdropEntries(e.lootdrop_id);
				int  slot              = 0;

				for (auto &f: loot_drop_entries) {
					int                rolled_count = npc->GetRolledItemCount(f.item_id);
					const EQ::ItemData *item        = database.GetItem(f.item_id);

					auto rolled_percentage = (float) ((float) ((float) rolled_count / (float) iterations) * 100);

					nlohmann::json drop;
					drop["slot"]                       = slot;
					drop["item_id"]                    = f.item_id;
					drop["item_name"]                  = item->Name;
					drop["chance"]                     = fmt::format("{:.2f}", f.chance);
					drop["simulate_rolled_count"]      = rolled_count;
					drop["simulate_rolled_percentage"] = fmt::format("{:.2f}", rolled_percentage);
					jle["drops"].push_back(drop);
					slot++;
				}

				j["global"]["lootdrops"].push_back(jle);
			}
		}
		j["data"]["time"] = benchmark.elapsed();
		res.status = 200;
		res.set_content(j.dump(), "application/json");
	}
	else {
		res.status = 400;
		j["error"] = fmt::format("Failed to spawn NPC ID [{}]", npc_id);
		res.set_content(j.dump(), "application/json");
	}
}
