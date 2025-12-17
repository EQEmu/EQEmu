/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://docs.eqemu.dev/developer/repositories
 */

#pragma once

#include "common/database.h"
#include "common/strings.h"

#include <ctime>

class BaseItemsEvolvingDetailsRepository {
public:
	struct ItemsEvolvingDetails {
		uint32_t    id;
		uint32_t    item_evo_id;
		uint32_t    item_evolve_level;
		uint32_t    item_id;
		uint32_t    type;
		std::string sub_type;
		int64_t     required_amount;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"item_evo_id",
			"item_evolve_level",
			"item_id",
			"type",
			"sub_type",
			"required_amount",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"item_evo_id",
			"item_evolve_level",
			"item_id",
			"type",
			"sub_type",
			"required_amount",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("items_evolving_details");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			SelectColumnsRaw(),
			TableName()
		);
	}

	static std::string BaseInsert()
	{
		return fmt::format(
			"INSERT INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static ItemsEvolvingDetails NewEntity()
	{
		ItemsEvolvingDetails e{};

		e.id                = 0;
		e.item_evo_id       = 0;
		e.item_evolve_level = 0;
		e.item_id           = 0;
		e.type              = 0;
		e.sub_type          = "0";
		e.required_amount   = 0;

		return e;
	}

	static ItemsEvolvingDetails GetItemsEvolvingDetails(
		const std::vector<ItemsEvolvingDetails> &items_evolving_detailss,
		int items_evolving_details_id
	)
	{
		for (auto &items_evolving_details : items_evolving_detailss) {
			if (items_evolving_details.id == items_evolving_details_id) {
				return items_evolving_details;
			}
		}

		return NewEntity();
	}

	static ItemsEvolvingDetails FindOne(
		Database& db,
		int items_evolving_details_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				items_evolving_details_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ItemsEvolvingDetails e{};

			e.id                = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.item_evo_id       = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_evolve_level = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_id           = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.type              = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.sub_type          = row[5] ? row[5] : "0";
			e.required_amount   = row[6] ? strtoll(row[6], nullptr, 10) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int items_evolving_details_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				items_evolving_details_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const ItemsEvolvingDetails &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.item_evo_id));
		v.push_back(columns[2] + " = " + std::to_string(e.item_evolve_level));
		v.push_back(columns[3] + " = " + std::to_string(e.item_id));
		v.push_back(columns[4] + " = " + std::to_string(e.type));
		v.push_back(columns[5] + " = '" + Strings::Escape(e.sub_type) + "'");
		v.push_back(columns[6] + " = " + std::to_string(e.required_amount));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ItemsEvolvingDetails InsertOne(
		Database& db,
		ItemsEvolvingDetails e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.item_evo_id));
		v.push_back(std::to_string(e.item_evolve_level));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.type));
		v.push_back("'" + Strings::Escape(e.sub_type) + "'");
		v.push_back(std::to_string(e.required_amount));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<ItemsEvolvingDetails> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.item_evo_id));
			v.push_back(std::to_string(e.item_evolve_level));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.type));
			v.push_back("'" + Strings::Escape(e.sub_type) + "'");
			v.push_back(std::to_string(e.required_amount));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<ItemsEvolvingDetails> All(Database& db)
	{
		std::vector<ItemsEvolvingDetails> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ItemsEvolvingDetails e{};

			e.id                = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.item_evo_id       = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_evolve_level = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_id           = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.type              = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.sub_type          = row[5] ? row[5] : "0";
			e.required_amount   = row[6] ? strtoll(row[6], nullptr, 10) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<ItemsEvolvingDetails> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<ItemsEvolvingDetails> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ItemsEvolvingDetails e{};

			e.id                = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.item_evo_id       = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_evolve_level = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_id           = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.type              = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.sub_type          = row[5] ? row[5] : "0";
			e.required_amount   = row[6] ? strtoll(row[6], nullptr, 10) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static std::string BaseReplace()
	{
		return fmt::format(
			"REPLACE INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static int ReplaceOne(
		Database& db,
		const ItemsEvolvingDetails &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.item_evo_id));
		v.push_back(std::to_string(e.item_evolve_level));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.type));
		v.push_back("'" + Strings::Escape(e.sub_type) + "'");
		v.push_back(std::to_string(e.required_amount));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseReplace(),
				Strings::Implode(",", v)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int ReplaceMany(
		Database& db,
		const std::vector<ItemsEvolvingDetails> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.item_evo_id));
			v.push_back(std::to_string(e.item_evolve_level));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.type));
			v.push_back("'" + Strings::Escape(e.sub_type) + "'");
			v.push_back(std::to_string(e.required_amount));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseReplace(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}
};
