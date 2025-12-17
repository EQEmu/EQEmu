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


class BaseMercArmorinfoRepository {
public:
	struct MercArmorinfo {
		int32_t  id;
		uint32_t merc_npc_type_id;
		uint8_t  minlevel;
		uint8_t  maxlevel;
		uint8_t  texture;
		uint8_t  helmtexture;
		uint32_t armortint_id;
		uint8_t  armortint_red;
		uint8_t  armortint_green;
		uint8_t  armortint_blue;
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
			"texture",
			"helmtexture",
			"armortint_id",
			"armortint_red",
			"armortint_green",
			"armortint_blue",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"merc_npc_type_id",
			"minlevel",
			"maxlevel",
			"texture",
			"helmtexture",
			"armortint_id",
			"armortint_red",
			"armortint_green",
			"armortint_blue",
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
		return std::string("merc_armorinfo");
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

	static MercArmorinfo NewEntity()
	{
		MercArmorinfo e{};

		e.id               = 0;
		e.merc_npc_type_id = 0;
		e.minlevel         = 1;
		e.maxlevel         = 255;
		e.texture          = 0;
		e.helmtexture      = 0;
		e.armortint_id     = 0;
		e.armortint_red    = 0;
		e.armortint_green  = 0;
		e.armortint_blue   = 0;

		return e;
	}

	static MercArmorinfo GetMercArmorinfo(
		const std::vector<MercArmorinfo> &merc_armorinfos,
		int merc_armorinfo_id
	)
	{
		for (auto &merc_armorinfo : merc_armorinfos) {
			if (merc_armorinfo.id == merc_armorinfo_id) {
				return merc_armorinfo;
			}
		}

		return NewEntity();
	}

	static MercArmorinfo FindOne(
		Database& db,
		int merc_armorinfo_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				merc_armorinfo_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			MercArmorinfo e{};

			e.id               = static_cast<int32_t>(atoi(row[0]));
			e.merc_npc_type_id = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.minlevel         = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.maxlevel         = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.texture          = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.helmtexture      = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.armortint_id     = static_cast<uint32_t>(strtoul(row[6], nullptr, 10));
			e.armortint_red    = static_cast<uint8_t>(strtoul(row[7], nullptr, 10));
			e.armortint_green  = static_cast<uint8_t>(strtoul(row[8], nullptr, 10));
			e.armortint_blue   = static_cast<uint8_t>(strtoul(row[9], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merc_armorinfo_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merc_armorinfo_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const MercArmorinfo &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.merc_npc_type_id));
		v.push_back(columns[2] + " = " + std::to_string(e.minlevel));
		v.push_back(columns[3] + " = " + std::to_string(e.maxlevel));
		v.push_back(columns[4] + " = " + std::to_string(e.texture));
		v.push_back(columns[5] + " = " + std::to_string(e.helmtexture));
		v.push_back(columns[6] + " = " + std::to_string(e.armortint_id));
		v.push_back(columns[7] + " = " + std::to_string(e.armortint_red));
		v.push_back(columns[8] + " = " + std::to_string(e.armortint_green));
		v.push_back(columns[9] + " = " + std::to_string(e.armortint_blue));

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

	static MercArmorinfo InsertOne(
		Database& db,
		MercArmorinfo e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.merc_npc_type_id));
		v.push_back(std::to_string(e.minlevel));
		v.push_back(std::to_string(e.maxlevel));
		v.push_back(std::to_string(e.texture));
		v.push_back(std::to_string(e.helmtexture));
		v.push_back(std::to_string(e.armortint_id));
		v.push_back(std::to_string(e.armortint_red));
		v.push_back(std::to_string(e.armortint_green));
		v.push_back(std::to_string(e.armortint_blue));

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
		const std::vector<MercArmorinfo> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.merc_npc_type_id));
			v.push_back(std::to_string(e.minlevel));
			v.push_back(std::to_string(e.maxlevel));
			v.push_back(std::to_string(e.texture));
			v.push_back(std::to_string(e.helmtexture));
			v.push_back(std::to_string(e.armortint_id));
			v.push_back(std::to_string(e.armortint_red));
			v.push_back(std::to_string(e.armortint_green));
			v.push_back(std::to_string(e.armortint_blue));

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

	static std::vector<MercArmorinfo> All(Database& db)
	{
		std::vector<MercArmorinfo> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercArmorinfo e{};

			e.id               = static_cast<int32_t>(atoi(row[0]));
			e.merc_npc_type_id = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.minlevel         = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.maxlevel         = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.texture          = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.helmtexture      = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.armortint_id     = static_cast<uint32_t>(strtoul(row[6], nullptr, 10));
			e.armortint_red    = static_cast<uint8_t>(strtoul(row[7], nullptr, 10));
			e.armortint_green  = static_cast<uint8_t>(strtoul(row[8], nullptr, 10));
			e.armortint_blue   = static_cast<uint8_t>(strtoul(row[9], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<MercArmorinfo> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<MercArmorinfo> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercArmorinfo e{};

			e.id               = static_cast<int32_t>(atoi(row[0]));
			e.merc_npc_type_id = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.minlevel         = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.maxlevel         = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.texture          = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.helmtexture      = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.armortint_id     = static_cast<uint32_t>(strtoul(row[6], nullptr, 10));
			e.armortint_red    = static_cast<uint8_t>(strtoul(row[7], nullptr, 10));
			e.armortint_green  = static_cast<uint8_t>(strtoul(row[8], nullptr, 10));
			e.armortint_blue   = static_cast<uint8_t>(strtoul(row[9], nullptr, 10));

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
		const MercArmorinfo &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.merc_npc_type_id));
		v.push_back(std::to_string(e.minlevel));
		v.push_back(std::to_string(e.maxlevel));
		v.push_back(std::to_string(e.texture));
		v.push_back(std::to_string(e.helmtexture));
		v.push_back(std::to_string(e.armortint_id));
		v.push_back(std::to_string(e.armortint_red));
		v.push_back(std::to_string(e.armortint_green));
		v.push_back(std::to_string(e.armortint_blue));

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
		const std::vector<MercArmorinfo> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.merc_npc_type_id));
			v.push_back(std::to_string(e.minlevel));
			v.push_back(std::to_string(e.maxlevel));
			v.push_back(std::to_string(e.texture));
			v.push_back(std::to_string(e.helmtexture));
			v.push_back(std::to_string(e.armortint_id));
			v.push_back(std::to_string(e.armortint_red));
			v.push_back(std::to_string(e.armortint_green));
			v.push_back(std::to_string(e.armortint_blue));

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
