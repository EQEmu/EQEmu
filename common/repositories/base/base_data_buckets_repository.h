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
#include <cereal/cereal.hpp>
class BaseDataBucketsRepository {
public:
	struct DataBuckets {
		uint64_t    id;
		std::string key_;
		std::string value;
		uint32_t    expires;
		uint64_t    account_id;
		uint64_t    character_id;
		uint32_t    npc_id;
		uint32_t    bot_id;
		uint16_t    zone_id;
		uint16_t    instance_id;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(id),
				CEREAL_NVP(key_),
				CEREAL_NVP(value),
				CEREAL_NVP(expires),
				CEREAL_NVP(account_id),
				CEREAL_NVP(character_id),
				CEREAL_NVP(npc_id),
				CEREAL_NVP(bot_id),
				CEREAL_NVP(zone_id),
				CEREAL_NVP(instance_id)
			);
		}
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"`key`",
			"value",
			"expires",
			"account_id",
			"character_id",
			"npc_id",
			"bot_id",
			"zone_id",
			"instance_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"`key`",
			"value",
			"expires",
			"account_id",
			"character_id",
			"npc_id",
			"bot_id",
			"zone_id",
			"instance_id",
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
		return std::string("data_buckets");
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

	static DataBuckets NewEntity()
	{
		DataBuckets e{};

		e.id           = 0;
		e.key_         = "";
		e.value        = "";
		e.expires      = 0;
		e.account_id   = 0;
		e.character_id = 0;
		e.npc_id       = 0;
		e.bot_id       = 0;
		e.zone_id      = 0;
		e.instance_id  = 0;

		return e;
	}

	static DataBuckets GetDataBuckets(
		const std::vector<DataBuckets> &data_bucketss,
		int data_buckets_id
	)
	{
		for (auto &data_buckets : data_bucketss) {
			if (data_buckets.id == data_buckets_id) {
				return data_buckets;
			}
		}

		return NewEntity();
	}

	static DataBuckets FindOne(
		Database& db,
		int data_buckets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				data_buckets_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			DataBuckets e{};

			e.id           = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.key_         = row[1] ? row[1] : "";
			e.value        = row[2] ? row[2] : "";
			e.expires      = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.account_id   = row[4] ? strtoull(row[4], nullptr, 10) : 0;
			e.character_id = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.npc_id       = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.bot_id       = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.zone_id      = row[8] ? static_cast<uint16_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.instance_id  = row[9] ? static_cast<uint16_t>(strtoul(row[9], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int data_buckets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				data_buckets_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const DataBuckets &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.key_) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.value) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.expires));
		v.push_back(columns[4] + " = " + std::to_string(e.account_id));
		v.push_back(columns[5] + " = " + std::to_string(e.character_id));
		v.push_back(columns[6] + " = " + std::to_string(e.npc_id));
		v.push_back(columns[7] + " = " + std::to_string(e.bot_id));
		v.push_back(columns[8] + " = " + std::to_string(e.zone_id));
		v.push_back(columns[9] + " = " + std::to_string(e.instance_id));

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

	static DataBuckets InsertOne(
		Database& db,
		DataBuckets e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.key_) + "'");
		v.push_back("'" + Strings::Escape(e.value) + "'");
		v.push_back(std::to_string(e.expires));
		v.push_back(std::to_string(e.account_id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.npc_id));
		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.instance_id));

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
		const std::vector<DataBuckets> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.key_) + "'");
			v.push_back("'" + Strings::Escape(e.value) + "'");
			v.push_back(std::to_string(e.expires));
			v.push_back(std::to_string(e.account_id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.npc_id));
			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.instance_id));

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

	static std::vector<DataBuckets> All(Database& db)
	{
		std::vector<DataBuckets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DataBuckets e{};

			e.id           = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.key_         = row[1] ? row[1] : "";
			e.value        = row[2] ? row[2] : "";
			e.expires      = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.account_id   = row[4] ? strtoull(row[4], nullptr, 10) : 0;
			e.character_id = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.npc_id       = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.bot_id       = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.zone_id      = row[8] ? static_cast<uint16_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.instance_id  = row[9] ? static_cast<uint16_t>(strtoul(row[9], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<DataBuckets> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<DataBuckets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DataBuckets e{};

			e.id           = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.key_         = row[1] ? row[1] : "";
			e.value        = row[2] ? row[2] : "";
			e.expires      = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.account_id   = row[4] ? strtoull(row[4], nullptr, 10) : 0;
			e.character_id = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.npc_id       = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.bot_id       = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.zone_id      = row[8] ? static_cast<uint16_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.instance_id  = row[9] ? static_cast<uint16_t>(strtoul(row[9], nullptr, 10)) : 0;

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
		const DataBuckets &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.key_) + "'");
		v.push_back("'" + Strings::Escape(e.value) + "'");
		v.push_back(std::to_string(e.expires));
		v.push_back(std::to_string(e.account_id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.npc_id));
		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.instance_id));

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
		const std::vector<DataBuckets> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.key_) + "'");
			v.push_back("'" + Strings::Escape(e.value) + "'");
			v.push_back(std::to_string(e.expires));
			v.push_back(std::to_string(e.account_id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.npc_id));
			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.instance_id));

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
