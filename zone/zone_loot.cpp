#include "zone.h"

#include "common/repositories/lootdrop_entries_repository.h"
#include "common/repositories/lootdrop_repository.h"
#include "common/repositories/loottable_entries_repository.h"
#include "common/repositories/loottable_repository.h"

#include <vector>

void Zone::LoadLootTables(const std::vector<uint32> in_loottable_ids)
{
	BenchTimer timer;

	// copy loottable_ids
	std::vector<uint32> loottable_ids = in_loottable_ids;

	// check if table is already loaded
	std::vector<uint32> loaded_tables = {};
	for (const auto &e: loottable_ids) {
		for (const auto &f: m_loottables) {
			if (e == f.id) {
				LogLootDetail("Loottable [{}] already loaded", e);
				loaded_tables.push_back(e);
			}
		}
	}

	// remove loaded tables from loottable_ids
	for (const auto &e: loaded_tables) {
		loottable_ids.erase(
			std::remove(
				loottable_ids.begin(),
				loottable_ids.end(),
				e
			),
			loottable_ids.end()
		);
	}

	if (loottable_ids.empty()) {
		LogLootDetail("No loottables to load");
		return;
	}

	auto loottables = LoottableRepository::GetWhere(
		content_db,
		fmt::format(
			"id IN ({})",
			Strings::Join(loottable_ids, ",")
		)
	);

	auto loottable_entries = LoottableEntriesRepository::GetWhere(
		content_db,
		fmt::format(
			"loottable_id IN ({})",
			Strings::Join(loottable_ids, ",")
		)
	);

	std::vector<uint32> lootdrop_ids;
	for (const auto     &e: loottable_entries) {
		if (std::find(
			lootdrop_ids.begin(),
			lootdrop_ids.end(),
			e.lootdrop_id
		) == lootdrop_ids.end()) {
			lootdrop_ids.push_back(e.lootdrop_id);
		}
	}

	if (lootdrop_ids.empty()) {
		LogLoot("No lootdrops to load for loottable(s) [{}]", Strings::Join(loottable_ids, ","));
		return;
	}

	auto lootdrops = LootdropRepository::GetWhere(
		content_db,
		fmt::format(
			"id IN ({})",
			Strings::Join(lootdrop_ids, ",")
		)
	);

	auto lootdrop_entries = LootdropEntriesRepository::GetWhere(
		content_db,
		fmt::format(
			"lootdrop_id IN ({})",
			Strings::Join(lootdrop_ids, ",")
		)
	);

	// emplace back tables to m_loottables if not exists
	for (const auto &e: loottables) {
		bool has_table = false;

		for (const auto &l: m_loottables) {
			if (e.id == l.id) {
				has_table = true;
				break;
			}
		}

		bool has_entry = false;

		if (!has_table) {
			// add loottable
			m_loottables.emplace_back(e);

			// add loottable entries
			for (const auto &f: loottable_entries) {
				if (e.id == f.loottable_id) {

					// check if loottable entry already exists in memory
					has_entry = false;
					for (const auto &g: m_loottable_entries) {
						if (f.loottable_id == g.loottable_id && f.lootdrop_id == g.lootdrop_id) {
							has_entry = true;
							break;
						}
					}

					if (!has_entry) {
						m_loottable_entries.emplace_back(f);
					}

					// add lootdrop
					for (const auto &g: lootdrops) {
						if (f.lootdrop_id == g.id) {

							// check if lootdrop already exists in memory
							has_entry = false;
							for (const auto &h: m_lootdrops) {
								if (g.id == h.id) {
									has_entry = true;
									break;
								}
							}

							if (!has_entry) {
								m_lootdrops.emplace_back(g);
							}

							// add lootdrop entries
							for (const auto &h: lootdrop_entries) {
								if (g.id == h.lootdrop_id) {

									// check if lootdrop entry already exists in memory
									has_entry = false;
									for (const auto &i: m_lootdrop_entries) {
										if (h.lootdrop_id == i.lootdrop_id && h.item_id == i.item_id) {
											has_entry = true;
											break;
										}
									}

									if (!has_entry) {
										m_lootdrop_entries.emplace_back(h);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (loottable_ids.size() > 1) {
		LogInfo("Loaded [{}] loottables ({}s)", m_loottables.size(), std::to_string(timer.elapsed()));
	}
}

void Zone::LoadLootTable(const uint32 loottable_id)
{
	if (loottable_id == 0) {
		return;
	}

	LoadLootTables({loottable_id});
}

void Zone::ClearLootTables()
{
	m_loottables.clear();
	m_loottable_entries.clear();
	m_lootdrops.clear();
	m_lootdrop_entries.clear();
}

void Zone::ReloadLootTables()
{
	ClearLootTables();

	std::vector<uint32> loottable_ids = {};
	for (const auto& n : entity_list.GetNPCList()) {
		// only add loottable if it's not already in the list
		if (n.second->GetLoottableID() != 0) {
			if (std::find(
				loottable_ids.begin(),
				loottable_ids.end(),
				n.second->GetLoottableID()
			) == loottable_ids.end()) {
				loottable_ids.push_back(n.second->GetLoottableID());
			}
		}
	}

	LoadLootTables(loottable_ids);
}

LoottableRepository::Loottable *Zone::GetLootTable(const uint32 loottable_id)
{
	for (auto &e: m_loottables) {
		if (e.id == loottable_id) {
			if (!WorldContentService::Instance()->DoesPassContentFiltering(
				ContentFlags{
					.min_expansion = e.min_expansion,
					.max_expansion = e.max_expansion,
					.content_flags = e.content_flags,
					.content_flags_disabled = e.content_flags_disabled
				}
			)) {
				LogLootDetail(
					"Loot table [{}] does not pass content filtering",
					loottable_id
				);
				continue;
			}

			return &e;
		}
	}

	return nullptr;
}

std::vector<LoottableEntriesRepository::LoottableEntries> Zone::GetLootTableEntries(const uint32 loottable_id) const
{
	std::vector<LoottableEntriesRepository::LoottableEntries> entries = {};
	for (const auto &e: m_loottable_entries) {
		if (e.loottable_id == loottable_id) {
			entries.emplace_back(e);
		}
	}

	return entries;
}

LootdropRepository::Lootdrop Zone::GetLootdrop(const uint32 lootdrop_id) const
{
	for (const auto &e: m_lootdrops) {
		if (e.id == lootdrop_id) {
			if (!WorldContentService::Instance()->DoesPassContentFiltering(
				ContentFlags{
					.min_expansion = e.min_expansion,
					.max_expansion = e.max_expansion,
					.content_flags = e.content_flags,
					.content_flags_disabled = e.content_flags_disabled
				}
			)) {
				LogLootDetail(
					"Lootdrop table [{}] does not pass content filtering",
					lootdrop_id
				);
				continue;
			}

			return e;
		}
	}

	return {};
}

std::vector<LootdropEntriesRepository::LootdropEntries> Zone::GetLootdropEntries(const uint32 lootdrop_id) const
{
	std::vector<LootdropEntriesRepository::LootdropEntries> entries = {};
	for (const auto &e: m_lootdrop_entries) {
		if (e.lootdrop_id == lootdrop_id) {
			if (!WorldContentService::Instance()->DoesPassContentFiltering(
				ContentFlags{
					.min_expansion = e.min_expansion,
					.max_expansion = e.max_expansion,
					.content_flags = e.content_flags,
					.content_flags_disabled = e.content_flags_disabled
				}
			)) {
				LogLootDetail(
					"Lootdrop [{}] Item [{}] ({}) does not pass content filtering",
					lootdrop_id,
					e.item_id,
					database.GetItem(e.item_id) ? database.GetItem(e.item_id)->Name : "Unknown"
				);
				continue;
			}

			entries.emplace_back(e);
		}
	}

	return entries;
}

void Zone::LoadLootDrops(const std::vector<uint32> in_lootdrop_ids)
{
	BenchTimer timer;

	// copy lootdrop_ids
	std::vector<uint32> lootdrop_ids = in_lootdrop_ids;

	// check if lootdrop is already loaded
	std::vector<uint32> loaded_drops = {};
	for (const auto &e: lootdrop_ids) {
		for (const auto &f: m_lootdrops) {
			if (e == f.id) {
				LogLootDetail("Lootdrop [{}] already loaded", e);
				loaded_drops.push_back(e);
			}
		}
	}

	// remove loaded drops from lootdrop_ids
	for (const auto &e: loaded_drops) {
		lootdrop_ids.erase(
			std::remove(
				lootdrop_ids.begin(),
				lootdrop_ids.end(),
				e
			),
			lootdrop_ids.end()
		);
	}

	if (lootdrop_ids.empty()) {
		LogLootDetail("No lootdrops to load");
		return;
	}

	auto lootdrops = LootdropRepository::GetWhere(
		content_db,
		fmt::format(
			"id IN ({})",
			Strings::Join(lootdrop_ids, ",")
		)
	);

	auto lootdrop_entries = LootdropEntriesRepository::GetWhere(
		content_db,
		fmt::format(
			"lootdrop_id IN ({})",
			Strings::Join(lootdrop_ids, ",")
		)
	);

	// emplace back drops to m_lootdrops if not exists
	for (const auto &e: lootdrops) {
		bool has_drop = false;

		for (const auto &l: m_lootdrops) {
			if (e.id == l.id) {
				has_drop = true;
				break;
			}
		}

		bool has_entry = false;
		if (!has_drop) {
			// add lootdrop
			m_lootdrops.emplace_back(e);

			// add lootdrop entries
			// add lootdrop entries
			for (const auto &h: lootdrop_entries) {
				if (e.id == h.lootdrop_id) {

					// check if lootdrop entry already exists in memory
					has_entry = false;
					for (const auto &i: m_lootdrop_entries) {
						if (h.lootdrop_id == i.lootdrop_id && h.item_id == i.item_id) {
							has_entry = true;
							break;
						}
					}

					if (!has_entry) {
						m_lootdrop_entries.emplace_back(h);
					}
				}
			}
		}
	}

	if (!lootdrop_ids.empty()) {
		LogInfo("Loaded [{}] lootdrops ({}s)", m_lootdrops.size(), std::to_string(timer.elapsed()));
	}
}
