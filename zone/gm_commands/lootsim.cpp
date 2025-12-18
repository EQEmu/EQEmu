#include "zone/client.h"

void command_lootsim(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (arguments < 3 || !sep->IsNumber(1) || !sep->IsNumber(2) || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Usage: #lootsim [npc_type_id] [loottable_id] [iterations] [loot_log_enabled=0]");
		return;
	}

	auto npc_id       = Strings::ToUnsignedInt(sep->arg[1]);
	auto loottable_id = Strings::ToUnsignedInt(sep->arg[2]);
	auto iterations   = Strings::ToUnsignedInt(sep->arg[3]) > 1000 ? 1000 : Strings::ToUnsignedInt(sep->arg[3]);
	auto log_enabled  = arguments > 3 ? Strings::ToUnsignedInt(sep->arg[4]) : false;

	// temporarily disable loot logging unless set explicitly
	EQEmuLogSys::Instance()->log_settings[Logs::Loot].log_to_console = log_enabled ? EQEmuLogSys::Instance()->log_settings[Logs::Loot].log_to_console : 0;
	EQEmuLogSys::Instance()->log_settings[Logs::Loot].log_to_file    = log_enabled ? EQEmuLogSys::Instance()->log_settings[Logs::Loot].log_to_file : 0;
	EQEmuLogSys::Instance()->log_settings[Logs::Loot].log_to_gmsay   = log_enabled ? EQEmuLogSys::Instance()->log_settings[Logs::Loot].log_to_gmsay : 0;

	auto npc_type = content_db.LoadNPCTypesData(npc_id);
	if (npc_type) {
		auto npc = new NPC(npc_type, nullptr, c->GetPosition(), GravityBehavior::Water);
		if (npc) {

			BenchTimer benchmark;

			npc->SetRecordLootStats(true);
			for (int i = 0; i < iterations; i++) {
				npc->AddLootTable(loottable_id);

				for (auto &id: zone->GetGlobalLootTables(npc)) {
					npc->AddLootTable(id);
				}
			}

			entity_list.AddNPC(npc);

			c->SendChatLineBreak();
			c->Message(
				Chat::White,
				fmt::format(
					"# [Loot Simulator] NPC [{}] ({}) Loot Table ID [{}] Dropped Items [{}] iterations [{}]",
					npc->GetCleanName(),
					npc_id,
					loottable_id,
					npc->GetRolledItems().size(),
					iterations
				).c_str()
			);
			c->SendChatLineBreak();

			// npc level loot table
			auto loot_table = zone->GetLootTable(loottable_id);
			if (!loot_table) {
				c->Message(Chat::Red, "Loot table not found");
				return;
			}

			auto le = zone->GetLootTableEntries(loottable_id);

			// translate above for loop using loot_table_entries
			for (auto &e: le) {
				c->Message(
					Chat::White,
					fmt::format(
						"# Lootdrop ID [{}] drop_limit [{}] min_drop [{}] mult [{}] probability [{}]",
						e.lootdrop_id,
						e.droplimit,
						e.mindrop,
						e.multiplier,
						e.probability
					).c_str()
				);

				auto loot_drop = zone->GetLootdrop(e.lootdrop_id);
				if (!loot_drop.id) {
					continue;
				}

				auto loot_drop_entries = zone->GetLootdropEntries(e.lootdrop_id);
				for (auto &f: loot_drop_entries) {
					int                rolled_count = npc->GetRolledItemCount(f.item_id);
					const EQ::ItemData *item        = database.GetItem(f.item_id);

					EQ::SayLinkEngine linker;
					linker.SetLinkType(EQ::saylink::SayLinkItemData);
					linker.SetItemData(item);

					auto rolled_percentage = (float) ((float) ((float) rolled_count / (float) iterations) * 100);

					c->Message(
						Chat::White,
						fmt::format(
							"-- lootdrop_id [{}] item_id [{}] chance [{}] rolled_count [{}] ({:.2f}%) name [{}]",
							f.lootdrop_id,
							f.item_id,
							f.chance,
							rolled_count,
							rolled_percentage,
							linker.GenerateLink()
						).c_str()
					);
				}
			}

			// global loot
			auto tables = zone->GetGlobalLootTables(npc);
			if (!tables.empty()) {
				c->SendChatLineBreak();
				c->Message(Chat::White, "# [Loot Simulator] Global Loot");
			}

			for (auto &id: tables) {
				c->SendChatLineBreak();
				c->Message(Chat::White, fmt::format("# Global Loot Table ID [{}]", id).c_str());
				c->SendChatLineBreak();

				loot_table = zone->GetLootTable(id);
				if (!loot_table) {
					c->Message(Chat::Red, fmt::format("Global Loot table not found [{}]", id).c_str());
					continue;
				}

				le = zone->GetLootTableEntries(id);

				// translate above for loop using loot_table_entries
				for (auto &e: le) {
					c->Message(
						Chat::White,
						fmt::format(
							"# Lootdrop ID [{}] drop_limit [{}] min_drop [{}] mult [{}] probability [{}]",
							e.lootdrop_id,
							e.droplimit,
							e.mindrop,
							e.multiplier,
							e.probability
						).c_str()
					);

					auto loot_drop = zone->GetLootdrop(e.lootdrop_id);
					if (!loot_drop.id) {
						continue;
					}

					auto loot_drop_entries = zone->GetLootdropEntries(e.lootdrop_id);
					for (auto &f: loot_drop_entries) {
						int                rolled_count = npc->GetRolledItemCount(f.item_id);
						const EQ::ItemData *item        = database.GetItem(f.item_id);

						EQ::SayLinkEngine linker;
						linker.SetLinkType(EQ::saylink::SayLinkItemData);
						linker.SetItemData(item);

						auto rolled_percentage = (float) ((float) ((float) rolled_count / (float) iterations) * 100);

						c->Message(
							Chat::White,
							fmt::format(
								"-- lootdrop_id [{}] item_id [{}] chance [{}] rolled_count [{}] ({:.2f}%) name [{}]",
								f.lootdrop_id,
								f.item_id,
								f.chance,
								rolled_count,
								rolled_percentage,
								linker.GenerateLink()
							).c_str()
						);
					}
				}
			}


			c->SendChatLineBreak();
			c->Message(
				Chat::White,
				fmt::format(
					"# Global Loot Benchmark End [{}] iterations took [{}](s)",
					iterations,
					benchmark.elapsed()
				).c_str()
			);
			c->SendChatLineBreak();

			EQEmuLogSys::Instance()->LoadLogDatabaseSettings();
		}
	}
	else {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to spawn NPC ID {}.",
				npc_id
			).c_str()
		);
	}
}


