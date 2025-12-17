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

class BaseCharacterParcelsRepository {
public:
	struct CharacterParcels {
		uint32_t    id;
		uint32_t    char_id;
		uint32_t    item_id;
		uint32_t    aug_slot_1;
		uint32_t    aug_slot_2;
		uint32_t    aug_slot_3;
		uint32_t    aug_slot_4;
		uint32_t    aug_slot_5;
		uint32_t    aug_slot_6;
		uint32_t    slot_id;
		uint32_t    quantity;
		uint32_t    evolve_amount;
		std::string from_name;
		std::string note;
		time_t      sent_date;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"char_id",
			"item_id",
			"aug_slot_1",
			"aug_slot_2",
			"aug_slot_3",
			"aug_slot_4",
			"aug_slot_5",
			"aug_slot_6",
			"slot_id",
			"quantity",
			"evolve_amount",
			"from_name",
			"note",
			"sent_date",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"char_id",
			"item_id",
			"aug_slot_1",
			"aug_slot_2",
			"aug_slot_3",
			"aug_slot_4",
			"aug_slot_5",
			"aug_slot_6",
			"slot_id",
			"quantity",
			"evolve_amount",
			"from_name",
			"note",
			"UNIX_TIMESTAMP(sent_date)",
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
		return std::string("character_parcels");
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

	static CharacterParcels NewEntity()
	{
		CharacterParcels e{};

		e.id            = 0;
		e.char_id       = 0;
		e.item_id       = 0;
		e.aug_slot_1    = 0;
		e.aug_slot_2    = 0;
		e.aug_slot_3    = 0;
		e.aug_slot_4    = 0;
		e.aug_slot_5    = 0;
		e.aug_slot_6    = 0;
		e.slot_id       = 0;
		e.quantity      = 0;
		e.evolve_amount = 0;
		e.from_name     = "";
		e.note          = "";
		e.sent_date     = 0;

		return e;
	}

	static CharacterParcels GetCharacterParcels(
		const std::vector<CharacterParcels> &character_parcelss,
		int character_parcels_id
	)
	{
		for (auto &character_parcels : character_parcelss) {
			if (character_parcels.id == character_parcels_id) {
				return character_parcels;
			}
		}

		return NewEntity();
	}

	static CharacterParcels FindOne(
		Database& db,
		int character_parcels_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_parcels_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterParcels e{};

			e.id            = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.char_id       = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_id       = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.aug_slot_1    = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.aug_slot_2    = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.aug_slot_3    = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.aug_slot_4    = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.aug_slot_5    = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.aug_slot_6    = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.slot_id       = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.quantity      = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.evolve_amount = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.from_name     = row[12] ? row[12] : "";
			e.note          = row[13] ? row[13] : "";
			e.sent_date     = strtoll(row[14] ? row[14] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_parcels_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_parcels_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterParcels &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.char_id));
		v.push_back(columns[2] + " = " + std::to_string(e.item_id));
		v.push_back(columns[3] + " = " + std::to_string(e.aug_slot_1));
		v.push_back(columns[4] + " = " + std::to_string(e.aug_slot_2));
		v.push_back(columns[5] + " = " + std::to_string(e.aug_slot_3));
		v.push_back(columns[6] + " = " + std::to_string(e.aug_slot_4));
		v.push_back(columns[7] + " = " + std::to_string(e.aug_slot_5));
		v.push_back(columns[8] + " = " + std::to_string(e.aug_slot_6));
		v.push_back(columns[9] + " = " + std::to_string(e.slot_id));
		v.push_back(columns[10] + " = " + std::to_string(e.quantity));
		v.push_back(columns[11] + " = " + std::to_string(e.evolve_amount));
		v.push_back(columns[12] + " = '" + Strings::Escape(e.from_name) + "'");
		v.push_back(columns[13] + " = '" + Strings::Escape(e.note) + "'");
		v.push_back(columns[14] + " = FROM_UNIXTIME(" + (e.sent_date > 0 ? std::to_string(e.sent_date) : "null") + ")");

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

	static CharacterParcels InsertOne(
		Database& db,
		CharacterParcels e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.aug_slot_1));
		v.push_back(std::to_string(e.aug_slot_2));
		v.push_back(std::to_string(e.aug_slot_3));
		v.push_back(std::to_string(e.aug_slot_4));
		v.push_back(std::to_string(e.aug_slot_5));
		v.push_back(std::to_string(e.aug_slot_6));
		v.push_back(std::to_string(e.slot_id));
		v.push_back(std::to_string(e.quantity));
		v.push_back(std::to_string(e.evolve_amount));
		v.push_back("'" + Strings::Escape(e.from_name) + "'");
		v.push_back("'" + Strings::Escape(e.note) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.sent_date > 0 ? std::to_string(e.sent_date) : "null") + ")");

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
		const std::vector<CharacterParcels> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.char_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.aug_slot_1));
			v.push_back(std::to_string(e.aug_slot_2));
			v.push_back(std::to_string(e.aug_slot_3));
			v.push_back(std::to_string(e.aug_slot_4));
			v.push_back(std::to_string(e.aug_slot_5));
			v.push_back(std::to_string(e.aug_slot_6));
			v.push_back(std::to_string(e.slot_id));
			v.push_back(std::to_string(e.quantity));
			v.push_back(std::to_string(e.evolve_amount));
			v.push_back("'" + Strings::Escape(e.from_name) + "'");
			v.push_back("'" + Strings::Escape(e.note) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.sent_date > 0 ? std::to_string(e.sent_date) : "null") + ")");

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

	static std::vector<CharacterParcels> All(Database& db)
	{
		std::vector<CharacterParcels> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterParcels e{};

			e.id            = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.char_id       = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_id       = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.aug_slot_1    = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.aug_slot_2    = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.aug_slot_3    = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.aug_slot_4    = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.aug_slot_5    = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.aug_slot_6    = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.slot_id       = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.quantity      = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.evolve_amount = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.from_name     = row[12] ? row[12] : "";
			e.note          = row[13] ? row[13] : "";
			e.sent_date     = strtoll(row[14] ? row[14] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterParcels> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterParcels> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterParcels e{};

			e.id            = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.char_id       = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_id       = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.aug_slot_1    = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.aug_slot_2    = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.aug_slot_3    = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.aug_slot_4    = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.aug_slot_5    = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.aug_slot_6    = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.slot_id       = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.quantity      = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.evolve_amount = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.from_name     = row[12] ? row[12] : "";
			e.note          = row[13] ? row[13] : "";
			e.sent_date     = strtoll(row[14] ? row[14] : "-1", nullptr, 10);

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
		const CharacterParcels &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.aug_slot_1));
		v.push_back(std::to_string(e.aug_slot_2));
		v.push_back(std::to_string(e.aug_slot_3));
		v.push_back(std::to_string(e.aug_slot_4));
		v.push_back(std::to_string(e.aug_slot_5));
		v.push_back(std::to_string(e.aug_slot_6));
		v.push_back(std::to_string(e.slot_id));
		v.push_back(std::to_string(e.quantity));
		v.push_back(std::to_string(e.evolve_amount));
		v.push_back("'" + Strings::Escape(e.from_name) + "'");
		v.push_back("'" + Strings::Escape(e.note) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.sent_date > 0 ? std::to_string(e.sent_date) : "null") + ")");

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
		const std::vector<CharacterParcels> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.char_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.aug_slot_1));
			v.push_back(std::to_string(e.aug_slot_2));
			v.push_back(std::to_string(e.aug_slot_3));
			v.push_back(std::to_string(e.aug_slot_4));
			v.push_back(std::to_string(e.aug_slot_5));
			v.push_back(std::to_string(e.aug_slot_6));
			v.push_back(std::to_string(e.slot_id));
			v.push_back(std::to_string(e.quantity));
			v.push_back(std::to_string(e.evolve_amount));
			v.push_back("'" + Strings::Escape(e.from_name) + "'");
			v.push_back("'" + Strings::Escape(e.note) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.sent_date > 0 ? std::to_string(e.sent_date) : "null") + ")");

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
