#include "global_loot_manager.h"

#include "zone/client.h"
#include "zone/dialogue_window.h"
#include "zone/npc.h"
#include "zone/zone.h"

extern Zone *zone;

std::vector<int> GlobalLootManager::GetGlobalLootTables(NPC *mob) const
{
	// we may be able to add a cache here if performance is an issue, but for now
	// just return NRVO'd vector
	// The cache would have to be keyed by NPCType and level (for NPCs with Max Level set)
	std::vector<int> tables;

	for (auto &e : m_entries) {
		if (e.PassesRules(mob)) {
			tables.push_back(e.GetLootTableID());
		}
	}

	return tables;
}

void GlobalLootManager::ShowZoneGlobalLoot(Client *c) const
{
	std::string global_loot_table;

	global_loot_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}{}",
			DialogueWindow::TableCell("ID"),
			DialogueWindow::TableCell("Table Name"),
			DialogueWindow::TableCell("Loottable ID")
		)
	);

	for (auto &e : m_entries) {
		global_loot_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}{}",
				DialogueWindow::TableCell(std::to_string(e.GetID())),
				DialogueWindow::TableCell(e.GetDescription()),
				DialogueWindow::TableCell(std::to_string(e.GetLootTableID()))
			)
		);
	}

	global_loot_table = DialogueWindow::Table(global_loot_table);

	c->SendPopupToClient(
		fmt::format(
			"Global Loot for {} ({})",
			zone->GetLongName(),
			zone->GetZoneID()
		).c_str(),
		global_loot_table.c_str()
	);
}

void GlobalLootManager::ShowNPCGlobalLoot(Client *c, NPC *t) const
{
	std::string global_loot_table;

	global_loot_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}{}",
			DialogueWindow::TableCell("ID"),
			DialogueWindow::TableCell("Table Name"),
			DialogueWindow::TableCell("Loottable ID")
		)
	);

	for (auto &e : m_entries) {
		if (e.PassesRules(t)) {
			global_loot_table += DialogueWindow::TableRow(
				fmt::format(
					"{}{}{}",
					DialogueWindow::TableCell(std::to_string(e.GetID())),
					DialogueWindow::TableCell(e.GetDescription()),
					DialogueWindow::TableCell(std::to_string(e.GetLootTableID()))
				)
			);
		}
	}

	global_loot_table = DialogueWindow::Table(global_loot_table);

	c->SendPopupToClient(
		fmt::format(
			"Global Loot for {}",
			c->GetTargetDescription(t)
		).c_str(),
		global_loot_table.c_str()
	);
}

bool GlobalLootEntry::PassesRules(NPC *mob) const
{
	bool bRace = false;
	bool bPassesRace = false;
	bool bBodyType = false;
	bool bPassesBodyType = false;
	bool bClass = false;
	bool bPassesClass = false;

	for (auto &r : m_rules) {
		switch (r.type) {
		case GlobalLoot::RuleTypes::LevelMin:
			if (mob->GetLevel() < r.value)
				return false;
			break;
		case GlobalLoot::RuleTypes::LevelMax:
			if (mob->GetLevel() > r.value)
				return false;
			break;
		case GlobalLoot::RuleTypes::Raid: // value == 0 must not be raid, value != 0 must be raid
			if (mob->IsRaidTarget() && !r.value)
				return false;
			if (!mob->IsRaidTarget() && r.value)
				return false;
			break;
		case GlobalLoot::RuleTypes::Rare:
			if (mob->IsRareSpawn() && !r.value)
				return false;
			if (!mob->IsRareSpawn() && r.value)
				return false;
			break;
		case GlobalLoot::RuleTypes::Race: // can have multiple races per rule set
			bRace = true; // we must pass race
			if (mob->GetRace() == r.value)
				bPassesRace = true;
			break;
		case GlobalLoot::RuleTypes::Class: // can have multiple classes per rule set
			bClass = true; // we must pass class
			if (mob->GetClass() == r.value)
				bPassesClass = true;
			break;
		case GlobalLoot::RuleTypes::BodyType: // can have multiple bodytypes per rule set
			bBodyType = true; // we must pass BodyType
			if (mob->GetBodyType() == r.value)
				bPassesBodyType = true;
			break;
		case GlobalLoot::RuleTypes::HotZone: // value == 0 must not be hot_zone, value != must be hot_zone
			if (zone->IsHotzone() && !r.value)
				return false;
			if (!zone->IsHotzone() && r.value)
				return false;
			break;
		default:
			break;
		}
	}

	if (bRace && !bPassesRace)
		return false;

	if (bClass && !bPassesClass)
		return false;

	if (bBodyType && !bPassesBodyType)
		return false;

	// we abort as early as possible if we fail a rule, so if we get here, we passed
	return true;
}

