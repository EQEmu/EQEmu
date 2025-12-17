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

class BaseAdventureStatsRepository {
public:
	struct AdventureStats {
		uint32_t player_id;
		uint32_t guk_wins;
		uint32_t mir_wins;
		uint32_t mmc_wins;
		uint32_t ruj_wins;
		uint32_t tak_wins;
		uint32_t guk_losses;
		uint32_t mir_losses;
		uint32_t mmc_losses;
		uint32_t ruj_losses;
		uint32_t tak_losses;
	};

	static std::string PrimaryKey()
	{
		return std::string("player_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"player_id",
			"guk_wins",
			"mir_wins",
			"mmc_wins",
			"ruj_wins",
			"tak_wins",
			"guk_losses",
			"mir_losses",
			"mmc_losses",
			"ruj_losses",
			"tak_losses",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"player_id",
			"guk_wins",
			"mir_wins",
			"mmc_wins",
			"ruj_wins",
			"tak_wins",
			"guk_losses",
			"mir_losses",
			"mmc_losses",
			"ruj_losses",
			"tak_losses",
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
		return std::string("adventure_stats");
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

	static AdventureStats NewEntity()
	{
		AdventureStats e{};

		e.player_id  = 0;
		e.guk_wins   = 0;
		e.mir_wins   = 0;
		e.mmc_wins   = 0;
		e.ruj_wins   = 0;
		e.tak_wins   = 0;
		e.guk_losses = 0;
		e.mir_losses = 0;
		e.mmc_losses = 0;
		e.ruj_losses = 0;
		e.tak_losses = 0;

		return e;
	}

	static AdventureStats GetAdventureStats(
		const std::vector<AdventureStats> &adventure_statss,
		int adventure_stats_id
	)
	{
		for (auto &adventure_stats : adventure_statss) {
			if (adventure_stats.player_id == adventure_stats_id) {
				return adventure_stats;
			}
		}

		return NewEntity();
	}

	static AdventureStats FindOne(
		Database& db,
		int adventure_stats_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				adventure_stats_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AdventureStats e{};

			e.player_id  = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.guk_wins   = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.mir_wins   = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.mmc_wins   = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.ruj_wins   = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.tak_wins   = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.guk_losses = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.mir_losses = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.mmc_losses = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.ruj_losses = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.tak_losses = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int adventure_stats_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				adventure_stats_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const AdventureStats &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.player_id));
		v.push_back(columns[1] + " = " + std::to_string(e.guk_wins));
		v.push_back(columns[2] + " = " + std::to_string(e.mir_wins));
		v.push_back(columns[3] + " = " + std::to_string(e.mmc_wins));
		v.push_back(columns[4] + " = " + std::to_string(e.ruj_wins));
		v.push_back(columns[5] + " = " + std::to_string(e.tak_wins));
		v.push_back(columns[6] + " = " + std::to_string(e.guk_losses));
		v.push_back(columns[7] + " = " + std::to_string(e.mir_losses));
		v.push_back(columns[8] + " = " + std::to_string(e.mmc_losses));
		v.push_back(columns[9] + " = " + std::to_string(e.ruj_losses));
		v.push_back(columns[10] + " = " + std::to_string(e.tak_losses));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.player_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AdventureStats InsertOne(
		Database& db,
		AdventureStats e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.player_id));
		v.push_back(std::to_string(e.guk_wins));
		v.push_back(std::to_string(e.mir_wins));
		v.push_back(std::to_string(e.mmc_wins));
		v.push_back(std::to_string(e.ruj_wins));
		v.push_back(std::to_string(e.tak_wins));
		v.push_back(std::to_string(e.guk_losses));
		v.push_back(std::to_string(e.mir_losses));
		v.push_back(std::to_string(e.mmc_losses));
		v.push_back(std::to_string(e.ruj_losses));
		v.push_back(std::to_string(e.tak_losses));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.player_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<AdventureStats> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.player_id));
			v.push_back(std::to_string(e.guk_wins));
			v.push_back(std::to_string(e.mir_wins));
			v.push_back(std::to_string(e.mmc_wins));
			v.push_back(std::to_string(e.ruj_wins));
			v.push_back(std::to_string(e.tak_wins));
			v.push_back(std::to_string(e.guk_losses));
			v.push_back(std::to_string(e.mir_losses));
			v.push_back(std::to_string(e.mmc_losses));
			v.push_back(std::to_string(e.ruj_losses));
			v.push_back(std::to_string(e.tak_losses));

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

	static std::vector<AdventureStats> All(Database& db)
	{
		std::vector<AdventureStats> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureStats e{};

			e.player_id  = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.guk_wins   = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.mir_wins   = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.mmc_wins   = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.ruj_wins   = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.tak_wins   = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.guk_losses = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.mir_losses = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.mmc_losses = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.ruj_losses = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.tak_losses = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<AdventureStats> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<AdventureStats> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureStats e{};

			e.player_id  = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.guk_wins   = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.mir_wins   = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.mmc_wins   = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.ruj_wins   = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.tak_wins   = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.guk_losses = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.mir_losses = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.mmc_losses = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.ruj_losses = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.tak_losses = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;

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
		const AdventureStats &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.player_id));
		v.push_back(std::to_string(e.guk_wins));
		v.push_back(std::to_string(e.mir_wins));
		v.push_back(std::to_string(e.mmc_wins));
		v.push_back(std::to_string(e.ruj_wins));
		v.push_back(std::to_string(e.tak_wins));
		v.push_back(std::to_string(e.guk_losses));
		v.push_back(std::to_string(e.mir_losses));
		v.push_back(std::to_string(e.mmc_losses));
		v.push_back(std::to_string(e.ruj_losses));
		v.push_back(std::to_string(e.tak_losses));

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
		const std::vector<AdventureStats> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.player_id));
			v.push_back(std::to_string(e.guk_wins));
			v.push_back(std::to_string(e.mir_wins));
			v.push_back(std::to_string(e.mmc_wins));
			v.push_back(std::to_string(e.ruj_wins));
			v.push_back(std::to_string(e.tak_wins));
			v.push_back(std::to_string(e.guk_losses));
			v.push_back(std::to_string(e.mir_losses));
			v.push_back(std::to_string(e.mmc_losses));
			v.push_back(std::to_string(e.ruj_losses));
			v.push_back(std::to_string(e.tak_losses));

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
