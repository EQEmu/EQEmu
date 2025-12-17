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


class BaseMercWeaponinfoRepository {
public:
	struct MercWeaponinfo {
		int32_t id;
		int32_t merc_npc_type_id;
		uint8_t minlevel;
		uint8_t maxlevel;
		int32_t d_melee_texture1;
		int32_t d_melee_texture2;
		uint8_t prim_melee_type;
		uint8_t sec_melee_type;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"merc_npc_type_id",
			"minlevel",
			"maxlevel",
			"d_melee_texture1",
			"d_melee_texture2",
			"prim_melee_type",
			"sec_melee_type",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"merc_npc_type_id",
			"minlevel",
			"maxlevel",
			"d_melee_texture1",
			"d_melee_texture2",
			"prim_melee_type",
			"sec_melee_type",
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
		return std::string("merc_weaponinfo");
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

	static MercWeaponinfo NewEntity()
	{
		MercWeaponinfo e{};

		e.id               = 0;
		e.merc_npc_type_id = 0;
		e.minlevel         = 0;
		e.maxlevel         = 0;
		e.d_melee_texture1 = 0;
		e.d_melee_texture2 = 0;
		e.prim_melee_type  = 28;
		e.sec_melee_type   = 28;

		return e;
	}

	static MercWeaponinfo GetMercWeaponinfo(
		const std::vector<MercWeaponinfo> &merc_weaponinfos,
		int merc_weaponinfo_id
	)
	{
		for (auto &merc_weaponinfo : merc_weaponinfos) {
			if (merc_weaponinfo.id == merc_weaponinfo_id) {
				return merc_weaponinfo;
			}
		}

		return NewEntity();
	}

	static MercWeaponinfo FindOne(
		Database& db,
		int merc_weaponinfo_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				merc_weaponinfo_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			MercWeaponinfo e{};

			e.id               = static_cast<int32_t>(atoi(row[0]));
			e.merc_npc_type_id = static_cast<int32_t>(atoi(row[1]));
			e.minlevel         = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.maxlevel         = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.d_melee_texture1 = static_cast<int32_t>(atoi(row[4]));
			e.d_melee_texture2 = static_cast<int32_t>(atoi(row[5]));
			e.prim_melee_type  = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.sec_melee_type   = static_cast<uint8_t>(strtoul(row[7], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merc_weaponinfo_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merc_weaponinfo_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const MercWeaponinfo &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.merc_npc_type_id));
		v.push_back(columns[2] + " = " + std::to_string(e.minlevel));
		v.push_back(columns[3] + " = " + std::to_string(e.maxlevel));
		v.push_back(columns[4] + " = " + std::to_string(e.d_melee_texture1));
		v.push_back(columns[5] + " = " + std::to_string(e.d_melee_texture2));
		v.push_back(columns[6] + " = " + std::to_string(e.prim_melee_type));
		v.push_back(columns[7] + " = " + std::to_string(e.sec_melee_type));

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

	static MercWeaponinfo InsertOne(
		Database& db,
		MercWeaponinfo e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.merc_npc_type_id));
		v.push_back(std::to_string(e.minlevel));
		v.push_back(std::to_string(e.maxlevel));
		v.push_back(std::to_string(e.d_melee_texture1));
		v.push_back(std::to_string(e.d_melee_texture2));
		v.push_back(std::to_string(e.prim_melee_type));
		v.push_back(std::to_string(e.sec_melee_type));

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
		const std::vector<MercWeaponinfo> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.merc_npc_type_id));
			v.push_back(std::to_string(e.minlevel));
			v.push_back(std::to_string(e.maxlevel));
			v.push_back(std::to_string(e.d_melee_texture1));
			v.push_back(std::to_string(e.d_melee_texture2));
			v.push_back(std::to_string(e.prim_melee_type));
			v.push_back(std::to_string(e.sec_melee_type));

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

	static std::vector<MercWeaponinfo> All(Database& db)
	{
		std::vector<MercWeaponinfo> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercWeaponinfo e{};

			e.id               = static_cast<int32_t>(atoi(row[0]));
			e.merc_npc_type_id = static_cast<int32_t>(atoi(row[1]));
			e.minlevel         = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.maxlevel         = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.d_melee_texture1 = static_cast<int32_t>(atoi(row[4]));
			e.d_melee_texture2 = static_cast<int32_t>(atoi(row[5]));
			e.prim_melee_type  = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.sec_melee_type   = static_cast<uint8_t>(strtoul(row[7], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<MercWeaponinfo> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<MercWeaponinfo> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercWeaponinfo e{};

			e.id               = static_cast<int32_t>(atoi(row[0]));
			e.merc_npc_type_id = static_cast<int32_t>(atoi(row[1]));
			e.minlevel         = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.maxlevel         = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.d_melee_texture1 = static_cast<int32_t>(atoi(row[4]));
			e.d_melee_texture2 = static_cast<int32_t>(atoi(row[5]));
			e.prim_melee_type  = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.sec_melee_type   = static_cast<uint8_t>(strtoul(row[7], nullptr, 10));

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
		const MercWeaponinfo &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.merc_npc_type_id));
		v.push_back(std::to_string(e.minlevel));
		v.push_back(std::to_string(e.maxlevel));
		v.push_back(std::to_string(e.d_melee_texture1));
		v.push_back(std::to_string(e.d_melee_texture2));
		v.push_back(std::to_string(e.prim_melee_type));
		v.push_back(std::to_string(e.sec_melee_type));

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
		const std::vector<MercWeaponinfo> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.merc_npc_type_id));
			v.push_back(std::to_string(e.minlevel));
			v.push_back(std::to_string(e.maxlevel));
			v.push_back(std::to_string(e.d_melee_texture1));
			v.push_back(std::to_string(e.d_melee_texture2));
			v.push_back(std::to_string(e.prim_melee_type));
			v.push_back(std::to_string(e.sec_melee_type));

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
