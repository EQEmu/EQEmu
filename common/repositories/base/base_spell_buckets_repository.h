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

class BaseSpellBucketsRepository {
public:
	struct SpellBuckets {
		uint32_t    spell_id;
		std::string bucket_name;
		std::string bucket_value;
		uint8_t     bucket_comparison;
	};

	static std::string PrimaryKey()
	{
		return std::string("spell_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"spell_id",
			"bucket_name",
			"bucket_value",
			"bucket_comparison",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"spell_id",
			"bucket_name",
			"bucket_value",
			"bucket_comparison",
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
		return std::string("spell_buckets");
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

	static SpellBuckets NewEntity()
	{
		SpellBuckets e{};

		e.spell_id          = 0;
		e.bucket_name       = "";
		e.bucket_value      = "";
		e.bucket_comparison = 0;

		return e;
	}

	static SpellBuckets GetSpellBuckets(
		const std::vector<SpellBuckets> &spell_bucketss,
		int spell_buckets_id
	)
	{
		for (auto &spell_buckets : spell_bucketss) {
			if (spell_buckets.spell_id == spell_buckets_id) {
				return spell_buckets;
			}
		}

		return NewEntity();
	}

	static SpellBuckets FindOne(
		Database& db,
		int spell_buckets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				spell_buckets_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SpellBuckets e{};

			e.spell_id          = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.bucket_name       = row[1] ? row[1] : "";
			e.bucket_value      = row[2] ? row[2] : "";
			e.bucket_comparison = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int spell_buckets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spell_buckets_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const SpellBuckets &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.spell_id));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.bucket_name) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.bucket_value) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.bucket_comparison));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.spell_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SpellBuckets InsertOne(
		Database& db,
		SpellBuckets e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.spell_id));
		v.push_back("'" + Strings::Escape(e.bucket_name) + "'");
		v.push_back("'" + Strings::Escape(e.bucket_value) + "'");
		v.push_back(std::to_string(e.bucket_comparison));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.spell_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<SpellBuckets> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.spell_id));
			v.push_back("'" + Strings::Escape(e.bucket_name) + "'");
			v.push_back("'" + Strings::Escape(e.bucket_value) + "'");
			v.push_back(std::to_string(e.bucket_comparison));

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

	static std::vector<SpellBuckets> All(Database& db)
	{
		std::vector<SpellBuckets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpellBuckets e{};

			e.spell_id          = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.bucket_name       = row[1] ? row[1] : "";
			e.bucket_value      = row[2] ? row[2] : "";
			e.bucket_comparison = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<SpellBuckets> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<SpellBuckets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpellBuckets e{};

			e.spell_id          = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.bucket_name       = row[1] ? row[1] : "";
			e.bucket_value      = row[2] ? row[2] : "";
			e.bucket_comparison = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;

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
		const SpellBuckets &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.spell_id));
		v.push_back("'" + Strings::Escape(e.bucket_name) + "'");
		v.push_back("'" + Strings::Escape(e.bucket_value) + "'");
		v.push_back(std::to_string(e.bucket_comparison));

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
		const std::vector<SpellBuckets> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.spell_id));
			v.push_back("'" + Strings::Escape(e.bucket_name) + "'");
			v.push_back("'" + Strings::Escape(e.bucket_value) + "'");
			v.push_back(std::to_string(e.bucket_comparison));

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
