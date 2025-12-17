#pragma once

#include "common/repositories/base/base_items_repository.h"

#include "common/database.h"
#include "common/strings.h"

class ItemsRepository: public BaseItemsRepository {
public:
	struct Bazaar_Results {
		uint32      item_id;
		std::string name;
		bool        stackable;
		uint32      icon;
		uint32      stats;
	};

	static std::vector<int32> GetItemIDsBySearchCriteria(
		Database& db,
		std::string search_string,
		int query_limit = 0
	)
	{
		auto query = fmt::format(
			"SELECT `id` FROM {} WHERE LOWER(`name`) LIKE '%%{}%%' ORDER BY id ASC",
			TableName(),
			Strings::Escape(search_string)
		);

		if (query_limit >= 1) {
			query += fmt::format(" LIMIT {}", query_limit);
		}

		std::vector<int32> item_id_list;

		auto results = db.QueryDatabase(query);
		if (!results.Success() || !results.RowCount()) {
			return item_id_list;
		}

		for (auto row : results) {
			item_id_list.emplace_back(Strings::ToInt(row[0]));
		}

		return item_id_list;
	}

	static std::unordered_map<uint32, Bazaar_Results> GetItemsForBazaarSearch(
		Database& db,
		const std::vector<std::string> &search_ids,
		const std::string &name,
		const std::string &field_criteria_items,
		const std::string &where_criteria_items,
		const uint32       query_limit = 0
	)
	{
		auto query = fmt::format(
			"SELECT id, name, stackable, icon, {} "
			"FROM items "
			"WHERE `name` LIKE '%%{}%%' AND {} AND id IN({}) "
			"ORDER BY id ASC",
			field_criteria_items,
			Strings::Escape(name),
			where_criteria_items,
			Strings::Implode(",", search_ids)
		);

		if (query_limit >= 1) {
			query += fmt::format(" LIMIT {}", query_limit);
		}

		std::unordered_map<uint32, Bazaar_Results> item_list;

		auto results = db.QueryDatabase(query);
		if (!results.Success() || !results.RowCount()) {
			return item_list;
		}

		item_list.reserve(results.RowCount());
		for (auto row : results) {
			Bazaar_Results br{};
			br.item_id   = row[0]       ? static_cast<int32_t>(atoi(row[0])) : 0;
			br.name      = row[1]       ? row[1]                             : "";
			br.stackable = atoi(row[2]) ? true                               : false;
			br.icon      = row[3]       ? static_cast<int32_t>(atoi(row[3])) : 0;
			br.stats     = row[4]       ? static_cast<int32_t>(atoi(row[4])) : 0;

			item_list.emplace(br.item_id, br);
		}

		return item_list;
	}

};
