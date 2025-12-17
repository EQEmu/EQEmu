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

class BaseMerchantlistRepository {
public:
	struct Merchantlist {
		int32_t     merchantid;
		uint32_t    slot;
		int32_t     item;
		int16_t     faction_required;
		uint8_t     level_required;
		uint8_t     min_status;
		uint8_t     max_status;
		uint16_t    alt_currency_cost;
		int32_t     classes_required;
		int32_t     probability;
		std::string bucket_name;
		std::string bucket_value;
		uint8_t     bucket_comparison;
		int8_t      min_expansion;
		int8_t      max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("merchantid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"merchantid",
			"slot",
			"item",
			"faction_required",
			"level_required",
			"min_status",
			"max_status",
			"alt_currency_cost",
			"classes_required",
			"probability",
			"bucket_name",
			"bucket_value",
			"bucket_comparison",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"merchantid",
			"slot",
			"item",
			"faction_required",
			"level_required",
			"min_status",
			"max_status",
			"alt_currency_cost",
			"classes_required",
			"probability",
			"bucket_name",
			"bucket_value",
			"bucket_comparison",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
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
		return std::string("merchantlist");
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

	static Merchantlist NewEntity()
	{
		Merchantlist e{};

		e.merchantid             = 0;
		e.slot                   = 0;
		e.item                   = 0;
		e.faction_required       = -100;
		e.level_required         = 0;
		e.min_status             = 0;
		e.max_status             = 255;
		e.alt_currency_cost      = 0;
		e.classes_required       = 65535;
		e.probability            = 100;
		e.bucket_name            = "";
		e.bucket_value           = "";
		e.bucket_comparison      = 0;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static Merchantlist GetMerchantlist(
		const std::vector<Merchantlist> &merchantlists,
		int merchantlist_id
	)
	{
		for (auto &merchantlist : merchantlists) {
			if (merchantlist.merchantid == merchantlist_id) {
				return merchantlist;
			}
		}

		return NewEntity();
	}

	static Merchantlist FindOne(
		Database& db,
		int merchantlist_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				merchantlist_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Merchantlist e{};

			e.merchantid             = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.slot                   = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item                   = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.faction_required       = row[3] ? static_cast<int16_t>(atoi(row[3])) : -100;
			e.level_required         = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.min_status             = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.max_status             = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 255;
			e.alt_currency_cost      = row[7] ? static_cast<uint16_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.classes_required       = row[8] ? static_cast<int32_t>(atoi(row[8])) : 65535;
			e.probability            = row[9] ? static_cast<int32_t>(atoi(row[9])) : 100;
			e.bucket_name            = row[10] ? row[10] : "";
			e.bucket_value           = row[11] ? row[11] : "";
			e.bucket_comparison      = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.min_expansion          = row[13] ? static_cast<int8_t>(atoi(row[13])) : -1;
			e.max_expansion          = row[14] ? static_cast<int8_t>(atoi(row[14])) : -1;
			e.content_flags          = row[15] ? row[15] : "";
			e.content_flags_disabled = row[16] ? row[16] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merchantlist_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merchantlist_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Merchantlist &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.merchantid));
		v.push_back(columns[1] + " = " + std::to_string(e.slot));
		v.push_back(columns[2] + " = " + std::to_string(e.item));
		v.push_back(columns[3] + " = " + std::to_string(e.faction_required));
		v.push_back(columns[4] + " = " + std::to_string(e.level_required));
		v.push_back(columns[5] + " = " + std::to_string(e.min_status));
		v.push_back(columns[6] + " = " + std::to_string(e.max_status));
		v.push_back(columns[7] + " = " + std::to_string(e.alt_currency_cost));
		v.push_back(columns[8] + " = " + std::to_string(e.classes_required));
		v.push_back(columns[9] + " = " + std::to_string(e.probability));
		v.push_back(columns[10] + " = '" + Strings::Escape(e.bucket_name) + "'");
		v.push_back(columns[11] + " = '" + Strings::Escape(e.bucket_value) + "'");
		v.push_back(columns[12] + " = " + std::to_string(e.bucket_comparison));
		v.push_back(columns[13] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[14] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[15] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[16] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.merchantid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Merchantlist InsertOne(
		Database& db,
		Merchantlist e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merchantid));
		v.push_back(std::to_string(e.slot));
		v.push_back(std::to_string(e.item));
		v.push_back(std::to_string(e.faction_required));
		v.push_back(std::to_string(e.level_required));
		v.push_back(std::to_string(e.min_status));
		v.push_back(std::to_string(e.max_status));
		v.push_back(std::to_string(e.alt_currency_cost));
		v.push_back(std::to_string(e.classes_required));
		v.push_back(std::to_string(e.probability));
		v.push_back("'" + Strings::Escape(e.bucket_name) + "'");
		v.push_back("'" + Strings::Escape(e.bucket_value) + "'");
		v.push_back(std::to_string(e.bucket_comparison));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.merchantid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Merchantlist> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merchantid));
			v.push_back(std::to_string(e.slot));
			v.push_back(std::to_string(e.item));
			v.push_back(std::to_string(e.faction_required));
			v.push_back(std::to_string(e.level_required));
			v.push_back(std::to_string(e.min_status));
			v.push_back(std::to_string(e.max_status));
			v.push_back(std::to_string(e.alt_currency_cost));
			v.push_back(std::to_string(e.classes_required));
			v.push_back(std::to_string(e.probability));
			v.push_back("'" + Strings::Escape(e.bucket_name) + "'");
			v.push_back("'" + Strings::Escape(e.bucket_value) + "'");
			v.push_back(std::to_string(e.bucket_comparison));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static std::vector<Merchantlist> All(Database& db)
	{
		std::vector<Merchantlist> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Merchantlist e{};

			e.merchantid             = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.slot                   = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item                   = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.faction_required       = row[3] ? static_cast<int16_t>(atoi(row[3])) : -100;
			e.level_required         = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.min_status             = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.max_status             = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 255;
			e.alt_currency_cost      = row[7] ? static_cast<uint16_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.classes_required       = row[8] ? static_cast<int32_t>(atoi(row[8])) : 65535;
			e.probability            = row[9] ? static_cast<int32_t>(atoi(row[9])) : 100;
			e.bucket_name            = row[10] ? row[10] : "";
			e.bucket_value           = row[11] ? row[11] : "";
			e.bucket_comparison      = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.min_expansion          = row[13] ? static_cast<int8_t>(atoi(row[13])) : -1;
			e.max_expansion          = row[14] ? static_cast<int8_t>(atoi(row[14])) : -1;
			e.content_flags          = row[15] ? row[15] : "";
			e.content_flags_disabled = row[16] ? row[16] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Merchantlist> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Merchantlist> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Merchantlist e{};

			e.merchantid             = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.slot                   = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item                   = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.faction_required       = row[3] ? static_cast<int16_t>(atoi(row[3])) : -100;
			e.level_required         = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.min_status             = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.max_status             = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 255;
			e.alt_currency_cost      = row[7] ? static_cast<uint16_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.classes_required       = row[8] ? static_cast<int32_t>(atoi(row[8])) : 65535;
			e.probability            = row[9] ? static_cast<int32_t>(atoi(row[9])) : 100;
			e.bucket_name            = row[10] ? row[10] : "";
			e.bucket_value           = row[11] ? row[11] : "";
			e.bucket_comparison      = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.min_expansion          = row[13] ? static_cast<int8_t>(atoi(row[13])) : -1;
			e.max_expansion          = row[14] ? static_cast<int8_t>(atoi(row[14])) : -1;
			e.content_flags          = row[15] ? row[15] : "";
			e.content_flags_disabled = row[16] ? row[16] : "";

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
		const Merchantlist &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merchantid));
		v.push_back(std::to_string(e.slot));
		v.push_back(std::to_string(e.item));
		v.push_back(std::to_string(e.faction_required));
		v.push_back(std::to_string(e.level_required));
		v.push_back(std::to_string(e.min_status));
		v.push_back(std::to_string(e.max_status));
		v.push_back(std::to_string(e.alt_currency_cost));
		v.push_back(std::to_string(e.classes_required));
		v.push_back(std::to_string(e.probability));
		v.push_back("'" + Strings::Escape(e.bucket_name) + "'");
		v.push_back("'" + Strings::Escape(e.bucket_value) + "'");
		v.push_back(std::to_string(e.bucket_comparison));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
		const std::vector<Merchantlist> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merchantid));
			v.push_back(std::to_string(e.slot));
			v.push_back(std::to_string(e.item));
			v.push_back(std::to_string(e.faction_required));
			v.push_back(std::to_string(e.level_required));
			v.push_back(std::to_string(e.min_status));
			v.push_back(std::to_string(e.max_status));
			v.push_back(std::to_string(e.alt_currency_cost));
			v.push_back(std::to_string(e.classes_required));
			v.push_back(std::to_string(e.probability));
			v.push_back("'" + Strings::Escape(e.bucket_name) + "'");
			v.push_back("'" + Strings::Escape(e.bucket_value) + "'");
			v.push_back(std::to_string(e.bucket_comparison));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
