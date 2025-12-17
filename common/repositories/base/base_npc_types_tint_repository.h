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

class BaseNpcTypesTintRepository {
public:
	struct NpcTypesTint {
		uint32_t    id;
		std::string tint_set_name;
		uint8_t     red1h;
		uint8_t     grn1h;
		uint8_t     blu1h;
		uint8_t     red2c;
		uint8_t     grn2c;
		uint8_t     blu2c;
		uint8_t     red3a;
		uint8_t     grn3a;
		uint8_t     blu3a;
		uint8_t     red4b;
		uint8_t     grn4b;
		uint8_t     blu4b;
		uint8_t     red5g;
		uint8_t     grn5g;
		uint8_t     blu5g;
		uint8_t     red6l;
		uint8_t     grn6l;
		uint8_t     blu6l;
		uint8_t     red7f;
		uint8_t     grn7f;
		uint8_t     blu7f;
		uint8_t     red8x;
		uint8_t     grn8x;
		uint8_t     blu8x;
		uint8_t     red9x;
		uint8_t     grn9x;
		uint8_t     blu9x;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"tint_set_name",
			"red1h",
			"grn1h",
			"blu1h",
			"red2c",
			"grn2c",
			"blu2c",
			"red3a",
			"grn3a",
			"blu3a",
			"red4b",
			"grn4b",
			"blu4b",
			"red5g",
			"grn5g",
			"blu5g",
			"red6l",
			"grn6l",
			"blu6l",
			"red7f",
			"grn7f",
			"blu7f",
			"red8x",
			"grn8x",
			"blu8x",
			"red9x",
			"grn9x",
			"blu9x",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"tint_set_name",
			"red1h",
			"grn1h",
			"blu1h",
			"red2c",
			"grn2c",
			"blu2c",
			"red3a",
			"grn3a",
			"blu3a",
			"red4b",
			"grn4b",
			"blu4b",
			"red5g",
			"grn5g",
			"blu5g",
			"red6l",
			"grn6l",
			"blu6l",
			"red7f",
			"grn7f",
			"blu7f",
			"red8x",
			"grn8x",
			"blu8x",
			"red9x",
			"grn9x",
			"blu9x",
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
		return std::string("npc_types_tint");
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

	static NpcTypesTint NewEntity()
	{
		NpcTypesTint e{};

		e.id            = 0;
		e.tint_set_name = "";
		e.red1h         = 0;
		e.grn1h         = 0;
		e.blu1h         = 0;
		e.red2c         = 0;
		e.grn2c         = 0;
		e.blu2c         = 0;
		e.red3a         = 0;
		e.grn3a         = 0;
		e.blu3a         = 0;
		e.red4b         = 0;
		e.grn4b         = 0;
		e.blu4b         = 0;
		e.red5g         = 0;
		e.grn5g         = 0;
		e.blu5g         = 0;
		e.red6l         = 0;
		e.grn6l         = 0;
		e.blu6l         = 0;
		e.red7f         = 0;
		e.grn7f         = 0;
		e.blu7f         = 0;
		e.red8x         = 0;
		e.grn8x         = 0;
		e.blu8x         = 0;
		e.red9x         = 0;
		e.grn9x         = 0;
		e.blu9x         = 0;

		return e;
	}

	static NpcTypesTint GetNpcTypesTint(
		const std::vector<NpcTypesTint> &npc_types_tints,
		int npc_types_tint_id
	)
	{
		for (auto &npc_types_tint : npc_types_tints) {
			if (npc_types_tint.id == npc_types_tint_id) {
				return npc_types_tint;
			}
		}

		return NewEntity();
	}

	static NpcTypesTint FindOne(
		Database& db,
		int npc_types_tint_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				npc_types_tint_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcTypesTint e{};

			e.id            = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.tint_set_name = row[1] ? row[1] : "";
			e.red1h         = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.grn1h         = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.blu1h         = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.red2c         = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.grn2c         = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.blu2c         = row[7] ? static_cast<uint8_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.red3a         = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.grn3a         = row[9] ? static_cast<uint8_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.blu3a         = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.red4b         = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.grn4b         = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.blu4b         = row[13] ? static_cast<uint8_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.red5g         = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.grn5g         = row[15] ? static_cast<uint8_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.blu5g         = row[16] ? static_cast<uint8_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.red6l         = row[17] ? static_cast<uint8_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.grn6l         = row[18] ? static_cast<uint8_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.blu6l         = row[19] ? static_cast<uint8_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.red7f         = row[20] ? static_cast<uint8_t>(strtoul(row[20], nullptr, 10)) : 0;
			e.grn7f         = row[21] ? static_cast<uint8_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.blu7f         = row[22] ? static_cast<uint8_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.red8x         = row[23] ? static_cast<uint8_t>(strtoul(row[23], nullptr, 10)) : 0;
			e.grn8x         = row[24] ? static_cast<uint8_t>(strtoul(row[24], nullptr, 10)) : 0;
			e.blu8x         = row[25] ? static_cast<uint8_t>(strtoul(row[25], nullptr, 10)) : 0;
			e.red9x         = row[26] ? static_cast<uint8_t>(strtoul(row[26], nullptr, 10)) : 0;
			e.grn9x         = row[27] ? static_cast<uint8_t>(strtoul(row[27], nullptr, 10)) : 0;
			e.blu9x         = row[28] ? static_cast<uint8_t>(strtoul(row[28], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int npc_types_tint_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_types_tint_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const NpcTypesTint &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.tint_set_name) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.red1h));
		v.push_back(columns[3] + " = " + std::to_string(e.grn1h));
		v.push_back(columns[4] + " = " + std::to_string(e.blu1h));
		v.push_back(columns[5] + " = " + std::to_string(e.red2c));
		v.push_back(columns[6] + " = " + std::to_string(e.grn2c));
		v.push_back(columns[7] + " = " + std::to_string(e.blu2c));
		v.push_back(columns[8] + " = " + std::to_string(e.red3a));
		v.push_back(columns[9] + " = " + std::to_string(e.grn3a));
		v.push_back(columns[10] + " = " + std::to_string(e.blu3a));
		v.push_back(columns[11] + " = " + std::to_string(e.red4b));
		v.push_back(columns[12] + " = " + std::to_string(e.grn4b));
		v.push_back(columns[13] + " = " + std::to_string(e.blu4b));
		v.push_back(columns[14] + " = " + std::to_string(e.red5g));
		v.push_back(columns[15] + " = " + std::to_string(e.grn5g));
		v.push_back(columns[16] + " = " + std::to_string(e.blu5g));
		v.push_back(columns[17] + " = " + std::to_string(e.red6l));
		v.push_back(columns[18] + " = " + std::to_string(e.grn6l));
		v.push_back(columns[19] + " = " + std::to_string(e.blu6l));
		v.push_back(columns[20] + " = " + std::to_string(e.red7f));
		v.push_back(columns[21] + " = " + std::to_string(e.grn7f));
		v.push_back(columns[22] + " = " + std::to_string(e.blu7f));
		v.push_back(columns[23] + " = " + std::to_string(e.red8x));
		v.push_back(columns[24] + " = " + std::to_string(e.grn8x));
		v.push_back(columns[25] + " = " + std::to_string(e.blu8x));
		v.push_back(columns[26] + " = " + std::to_string(e.red9x));
		v.push_back(columns[27] + " = " + std::to_string(e.grn9x));
		v.push_back(columns[28] + " = " + std::to_string(e.blu9x));

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

	static NpcTypesTint InsertOne(
		Database& db,
		NpcTypesTint e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.tint_set_name) + "'");
		v.push_back(std::to_string(e.red1h));
		v.push_back(std::to_string(e.grn1h));
		v.push_back(std::to_string(e.blu1h));
		v.push_back(std::to_string(e.red2c));
		v.push_back(std::to_string(e.grn2c));
		v.push_back(std::to_string(e.blu2c));
		v.push_back(std::to_string(e.red3a));
		v.push_back(std::to_string(e.grn3a));
		v.push_back(std::to_string(e.blu3a));
		v.push_back(std::to_string(e.red4b));
		v.push_back(std::to_string(e.grn4b));
		v.push_back(std::to_string(e.blu4b));
		v.push_back(std::to_string(e.red5g));
		v.push_back(std::to_string(e.grn5g));
		v.push_back(std::to_string(e.blu5g));
		v.push_back(std::to_string(e.red6l));
		v.push_back(std::to_string(e.grn6l));
		v.push_back(std::to_string(e.blu6l));
		v.push_back(std::to_string(e.red7f));
		v.push_back(std::to_string(e.grn7f));
		v.push_back(std::to_string(e.blu7f));
		v.push_back(std::to_string(e.red8x));
		v.push_back(std::to_string(e.grn8x));
		v.push_back(std::to_string(e.blu8x));
		v.push_back(std::to_string(e.red9x));
		v.push_back(std::to_string(e.grn9x));
		v.push_back(std::to_string(e.blu9x));

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
		const std::vector<NpcTypesTint> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.tint_set_name) + "'");
			v.push_back(std::to_string(e.red1h));
			v.push_back(std::to_string(e.grn1h));
			v.push_back(std::to_string(e.blu1h));
			v.push_back(std::to_string(e.red2c));
			v.push_back(std::to_string(e.grn2c));
			v.push_back(std::to_string(e.blu2c));
			v.push_back(std::to_string(e.red3a));
			v.push_back(std::to_string(e.grn3a));
			v.push_back(std::to_string(e.blu3a));
			v.push_back(std::to_string(e.red4b));
			v.push_back(std::to_string(e.grn4b));
			v.push_back(std::to_string(e.blu4b));
			v.push_back(std::to_string(e.red5g));
			v.push_back(std::to_string(e.grn5g));
			v.push_back(std::to_string(e.blu5g));
			v.push_back(std::to_string(e.red6l));
			v.push_back(std::to_string(e.grn6l));
			v.push_back(std::to_string(e.blu6l));
			v.push_back(std::to_string(e.red7f));
			v.push_back(std::to_string(e.grn7f));
			v.push_back(std::to_string(e.blu7f));
			v.push_back(std::to_string(e.red8x));
			v.push_back(std::to_string(e.grn8x));
			v.push_back(std::to_string(e.blu8x));
			v.push_back(std::to_string(e.red9x));
			v.push_back(std::to_string(e.grn9x));
			v.push_back(std::to_string(e.blu9x));

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

	static std::vector<NpcTypesTint> All(Database& db)
	{
		std::vector<NpcTypesTint> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcTypesTint e{};

			e.id            = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.tint_set_name = row[1] ? row[1] : "";
			e.red1h         = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.grn1h         = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.blu1h         = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.red2c         = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.grn2c         = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.blu2c         = row[7] ? static_cast<uint8_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.red3a         = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.grn3a         = row[9] ? static_cast<uint8_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.blu3a         = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.red4b         = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.grn4b         = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.blu4b         = row[13] ? static_cast<uint8_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.red5g         = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.grn5g         = row[15] ? static_cast<uint8_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.blu5g         = row[16] ? static_cast<uint8_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.red6l         = row[17] ? static_cast<uint8_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.grn6l         = row[18] ? static_cast<uint8_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.blu6l         = row[19] ? static_cast<uint8_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.red7f         = row[20] ? static_cast<uint8_t>(strtoul(row[20], nullptr, 10)) : 0;
			e.grn7f         = row[21] ? static_cast<uint8_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.blu7f         = row[22] ? static_cast<uint8_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.red8x         = row[23] ? static_cast<uint8_t>(strtoul(row[23], nullptr, 10)) : 0;
			e.grn8x         = row[24] ? static_cast<uint8_t>(strtoul(row[24], nullptr, 10)) : 0;
			e.blu8x         = row[25] ? static_cast<uint8_t>(strtoul(row[25], nullptr, 10)) : 0;
			e.red9x         = row[26] ? static_cast<uint8_t>(strtoul(row[26], nullptr, 10)) : 0;
			e.grn9x         = row[27] ? static_cast<uint8_t>(strtoul(row[27], nullptr, 10)) : 0;
			e.blu9x         = row[28] ? static_cast<uint8_t>(strtoul(row[28], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<NpcTypesTint> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<NpcTypesTint> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcTypesTint e{};

			e.id            = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.tint_set_name = row[1] ? row[1] : "";
			e.red1h         = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.grn1h         = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.blu1h         = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.red2c         = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.grn2c         = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.blu2c         = row[7] ? static_cast<uint8_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.red3a         = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.grn3a         = row[9] ? static_cast<uint8_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.blu3a         = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.red4b         = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.grn4b         = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.blu4b         = row[13] ? static_cast<uint8_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.red5g         = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.grn5g         = row[15] ? static_cast<uint8_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.blu5g         = row[16] ? static_cast<uint8_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.red6l         = row[17] ? static_cast<uint8_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.grn6l         = row[18] ? static_cast<uint8_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.blu6l         = row[19] ? static_cast<uint8_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.red7f         = row[20] ? static_cast<uint8_t>(strtoul(row[20], nullptr, 10)) : 0;
			e.grn7f         = row[21] ? static_cast<uint8_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.blu7f         = row[22] ? static_cast<uint8_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.red8x         = row[23] ? static_cast<uint8_t>(strtoul(row[23], nullptr, 10)) : 0;
			e.grn8x         = row[24] ? static_cast<uint8_t>(strtoul(row[24], nullptr, 10)) : 0;
			e.blu8x         = row[25] ? static_cast<uint8_t>(strtoul(row[25], nullptr, 10)) : 0;
			e.red9x         = row[26] ? static_cast<uint8_t>(strtoul(row[26], nullptr, 10)) : 0;
			e.grn9x         = row[27] ? static_cast<uint8_t>(strtoul(row[27], nullptr, 10)) : 0;
			e.blu9x         = row[28] ? static_cast<uint8_t>(strtoul(row[28], nullptr, 10)) : 0;

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
		const NpcTypesTint &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.tint_set_name) + "'");
		v.push_back(std::to_string(e.red1h));
		v.push_back(std::to_string(e.grn1h));
		v.push_back(std::to_string(e.blu1h));
		v.push_back(std::to_string(e.red2c));
		v.push_back(std::to_string(e.grn2c));
		v.push_back(std::to_string(e.blu2c));
		v.push_back(std::to_string(e.red3a));
		v.push_back(std::to_string(e.grn3a));
		v.push_back(std::to_string(e.blu3a));
		v.push_back(std::to_string(e.red4b));
		v.push_back(std::to_string(e.grn4b));
		v.push_back(std::to_string(e.blu4b));
		v.push_back(std::to_string(e.red5g));
		v.push_back(std::to_string(e.grn5g));
		v.push_back(std::to_string(e.blu5g));
		v.push_back(std::to_string(e.red6l));
		v.push_back(std::to_string(e.grn6l));
		v.push_back(std::to_string(e.blu6l));
		v.push_back(std::to_string(e.red7f));
		v.push_back(std::to_string(e.grn7f));
		v.push_back(std::to_string(e.blu7f));
		v.push_back(std::to_string(e.red8x));
		v.push_back(std::to_string(e.grn8x));
		v.push_back(std::to_string(e.blu8x));
		v.push_back(std::to_string(e.red9x));
		v.push_back(std::to_string(e.grn9x));
		v.push_back(std::to_string(e.blu9x));

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
		const std::vector<NpcTypesTint> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.tint_set_name) + "'");
			v.push_back(std::to_string(e.red1h));
			v.push_back(std::to_string(e.grn1h));
			v.push_back(std::to_string(e.blu1h));
			v.push_back(std::to_string(e.red2c));
			v.push_back(std::to_string(e.grn2c));
			v.push_back(std::to_string(e.blu2c));
			v.push_back(std::to_string(e.red3a));
			v.push_back(std::to_string(e.grn3a));
			v.push_back(std::to_string(e.blu3a));
			v.push_back(std::to_string(e.red4b));
			v.push_back(std::to_string(e.grn4b));
			v.push_back(std::to_string(e.blu4b));
			v.push_back(std::to_string(e.red5g));
			v.push_back(std::to_string(e.grn5g));
			v.push_back(std::to_string(e.blu5g));
			v.push_back(std::to_string(e.red6l));
			v.push_back(std::to_string(e.grn6l));
			v.push_back(std::to_string(e.blu6l));
			v.push_back(std::to_string(e.red7f));
			v.push_back(std::to_string(e.grn7f));
			v.push_back(std::to_string(e.blu7f));
			v.push_back(std::to_string(e.red8x));
			v.push_back(std::to_string(e.grn8x));
			v.push_back(std::to_string(e.blu8x));
			v.push_back(std::to_string(e.red9x));
			v.push_back(std::to_string(e.grn9x));
			v.push_back(std::to_string(e.blu9x));

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
