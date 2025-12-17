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

class BaseBotHealRotationsRepository {
public:
	struct BotHealRotations {
		uint32_t heal_rotation_index;
		uint32_t bot_id;
		uint32_t interval_;
		uint32_t fast_heals;
		uint32_t adaptive_targeting;
		uint32_t casting_override;
		float    safe_hp_base;
		float    safe_hp_cloth;
		float    safe_hp_leather;
		float    safe_hp_chain;
		float    safe_hp_plate;
		float    critical_hp_base;
		float    critical_hp_cloth;
		float    critical_hp_leather;
		float    critical_hp_chain;
		float    critical_hp_plate;
	};

	static std::string PrimaryKey()
	{
		return std::string("heal_rotation_index");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"heal_rotation_index",
			"bot_id",
			"`interval`",
			"fast_heals",
			"adaptive_targeting",
			"casting_override",
			"safe_hp_base",
			"safe_hp_cloth",
			"safe_hp_leather",
			"safe_hp_chain",
			"safe_hp_plate",
			"critical_hp_base",
			"critical_hp_cloth",
			"critical_hp_leather",
			"critical_hp_chain",
			"critical_hp_plate",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"heal_rotation_index",
			"bot_id",
			"`interval`",
			"fast_heals",
			"adaptive_targeting",
			"casting_override",
			"safe_hp_base",
			"safe_hp_cloth",
			"safe_hp_leather",
			"safe_hp_chain",
			"safe_hp_plate",
			"critical_hp_base",
			"critical_hp_cloth",
			"critical_hp_leather",
			"critical_hp_chain",
			"critical_hp_plate",
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
		return std::string("bot_heal_rotations");
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

	static BotHealRotations NewEntity()
	{
		BotHealRotations e{};

		e.heal_rotation_index = 0;
		e.bot_id              = 0;
		e.interval_           = 0;
		e.fast_heals          = 0;
		e.adaptive_targeting  = 0;
		e.casting_override    = 0;
		e.safe_hp_base        = 0;
		e.safe_hp_cloth       = 0;
		e.safe_hp_leather     = 0;
		e.safe_hp_chain       = 0;
		e.safe_hp_plate       = 0;
		e.critical_hp_base    = 0;
		e.critical_hp_cloth   = 0;
		e.critical_hp_leather = 0;
		e.critical_hp_chain   = 0;
		e.critical_hp_plate   = 0;

		return e;
	}

	static BotHealRotations GetBotHealRotations(
		const std::vector<BotHealRotations> &bot_heal_rotationss,
		int bot_heal_rotations_id
	)
	{
		for (auto &bot_heal_rotations : bot_heal_rotationss) {
			if (bot_heal_rotations.heal_rotation_index == bot_heal_rotations_id) {
				return bot_heal_rotations;
			}
		}

		return NewEntity();
	}

	static BotHealRotations FindOne(
		Database& db,
		int bot_heal_rotations_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				bot_heal_rotations_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotHealRotations e{};

			e.heal_rotation_index = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.bot_id              = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.interval_           = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.fast_heals          = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.adaptive_targeting  = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.casting_override    = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_heal_rotations_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_heal_rotations_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotHealRotations &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.bot_id));
		v.push_back(columns[2] + " = " + std::to_string(e.interval_));
		v.push_back(columns[3] + " = " + std::to_string(e.fast_heals));
		v.push_back(columns[4] + " = " + std::to_string(e.adaptive_targeting));
		v.push_back(columns[5] + " = " + std::to_string(e.casting_override));
		v.push_back(columns[6] + " = " + std::to_string(e.safe_hp_base));
		v.push_back(columns[7] + " = " + std::to_string(e.safe_hp_cloth));
		v.push_back(columns[8] + " = " + std::to_string(e.safe_hp_leather));
		v.push_back(columns[9] + " = " + std::to_string(e.safe_hp_chain));
		v.push_back(columns[10] + " = " + std::to_string(e.safe_hp_plate));
		v.push_back(columns[11] + " = " + std::to_string(e.critical_hp_base));
		v.push_back(columns[12] + " = " + std::to_string(e.critical_hp_cloth));
		v.push_back(columns[13] + " = " + std::to_string(e.critical_hp_leather));
		v.push_back(columns[14] + " = " + std::to_string(e.critical_hp_chain));
		v.push_back(columns[15] + " = " + std::to_string(e.critical_hp_plate));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.heal_rotation_index
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static BotHealRotations InsertOne(
		Database& db,
		BotHealRotations e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.heal_rotation_index));
		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.interval_));
		v.push_back(std::to_string(e.fast_heals));
		v.push_back(std::to_string(e.adaptive_targeting));
		v.push_back(std::to_string(e.casting_override));
		v.push_back(std::to_string(e.safe_hp_base));
		v.push_back(std::to_string(e.safe_hp_cloth));
		v.push_back(std::to_string(e.safe_hp_leather));
		v.push_back(std::to_string(e.safe_hp_chain));
		v.push_back(std::to_string(e.safe_hp_plate));
		v.push_back(std::to_string(e.critical_hp_base));
		v.push_back(std::to_string(e.critical_hp_cloth));
		v.push_back(std::to_string(e.critical_hp_leather));
		v.push_back(std::to_string(e.critical_hp_chain));
		v.push_back(std::to_string(e.critical_hp_plate));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.heal_rotation_index = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<BotHealRotations> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.heal_rotation_index));
			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.interval_));
			v.push_back(std::to_string(e.fast_heals));
			v.push_back(std::to_string(e.adaptive_targeting));
			v.push_back(std::to_string(e.casting_override));
			v.push_back(std::to_string(e.safe_hp_base));
			v.push_back(std::to_string(e.safe_hp_cloth));
			v.push_back(std::to_string(e.safe_hp_leather));
			v.push_back(std::to_string(e.safe_hp_chain));
			v.push_back(std::to_string(e.safe_hp_plate));
			v.push_back(std::to_string(e.critical_hp_base));
			v.push_back(std::to_string(e.critical_hp_cloth));
			v.push_back(std::to_string(e.critical_hp_leather));
			v.push_back(std::to_string(e.critical_hp_chain));
			v.push_back(std::to_string(e.critical_hp_plate));

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

	static std::vector<BotHealRotations> All(Database& db)
	{
		std::vector<BotHealRotations> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotHealRotations e{};

			e.heal_rotation_index = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.bot_id              = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.interval_           = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.fast_heals          = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.adaptive_targeting  = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.casting_override    = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotHealRotations> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotHealRotations> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotHealRotations e{};

			e.heal_rotation_index = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.bot_id              = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.interval_           = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.fast_heals          = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.adaptive_targeting  = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.casting_override    = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;

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
		const BotHealRotations &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.heal_rotation_index));
		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.interval_));
		v.push_back(std::to_string(e.fast_heals));
		v.push_back(std::to_string(e.adaptive_targeting));
		v.push_back(std::to_string(e.casting_override));
		v.push_back(std::to_string(e.safe_hp_base));
		v.push_back(std::to_string(e.safe_hp_cloth));
		v.push_back(std::to_string(e.safe_hp_leather));
		v.push_back(std::to_string(e.safe_hp_chain));
		v.push_back(std::to_string(e.safe_hp_plate));
		v.push_back(std::to_string(e.critical_hp_base));
		v.push_back(std::to_string(e.critical_hp_cloth));
		v.push_back(std::to_string(e.critical_hp_leather));
		v.push_back(std::to_string(e.critical_hp_chain));
		v.push_back(std::to_string(e.critical_hp_plate));

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
		const std::vector<BotHealRotations> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.heal_rotation_index));
			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.interval_));
			v.push_back(std::to_string(e.fast_heals));
			v.push_back(std::to_string(e.adaptive_targeting));
			v.push_back(std::to_string(e.casting_override));
			v.push_back(std::to_string(e.safe_hp_base));
			v.push_back(std::to_string(e.safe_hp_cloth));
			v.push_back(std::to_string(e.safe_hp_leather));
			v.push_back(std::to_string(e.safe_hp_chain));
			v.push_back(std::to_string(e.safe_hp_plate));
			v.push_back(std::to_string(e.critical_hp_base));
			v.push_back(std::to_string(e.critical_hp_cloth));
			v.push_back(std::to_string(e.critical_hp_leather));
			v.push_back(std::to_string(e.critical_hp_chain));
			v.push_back(std::to_string(e.critical_hp_plate));

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
