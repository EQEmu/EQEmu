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

class BaseMercsRepository {
public:
	struct Mercs {
		uint32_t    MercID;
		uint32_t    OwnerCharacterID;
		uint8_t     Slot;
		std::string Name;
		uint32_t    TemplateID;
		uint32_t    SuspendedTime;
		uint8_t     IsSuspended;
		uint32_t    TimerRemaining;
		uint8_t     Gender;
		float       MercSize;
		uint8_t     StanceID;
		uint32_t    HP;
		uint32_t    Mana;
		uint32_t    Endurance;
		uint32_t    Face;
		uint32_t    LuclinHairStyle;
		uint32_t    LuclinHairColor;
		uint32_t    LuclinEyeColor;
		uint32_t    LuclinEyeColor2;
		uint32_t    LuclinBeardColor;
		uint32_t    LuclinBeard;
		uint32_t    DrakkinHeritage;
		uint32_t    DrakkinTattoo;
		uint32_t    DrakkinDetails;
	};

	static std::string PrimaryKey()
	{
		return std::string("MercID");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"MercID",
			"OwnerCharacterID",
			"Slot",
			"Name",
			"TemplateID",
			"SuspendedTime",
			"IsSuspended",
			"TimerRemaining",
			"Gender",
			"MercSize",
			"StanceID",
			"HP",
			"Mana",
			"Endurance",
			"Face",
			"LuclinHairStyle",
			"LuclinHairColor",
			"LuclinEyeColor",
			"LuclinEyeColor2",
			"LuclinBeardColor",
			"LuclinBeard",
			"DrakkinHeritage",
			"DrakkinTattoo",
			"DrakkinDetails",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"MercID",
			"OwnerCharacterID",
			"Slot",
			"Name",
			"TemplateID",
			"SuspendedTime",
			"IsSuspended",
			"TimerRemaining",
			"Gender",
			"MercSize",
			"StanceID",
			"HP",
			"Mana",
			"Endurance",
			"Face",
			"LuclinHairStyle",
			"LuclinHairColor",
			"LuclinEyeColor",
			"LuclinEyeColor2",
			"LuclinBeardColor",
			"LuclinBeard",
			"DrakkinHeritage",
			"DrakkinTattoo",
			"DrakkinDetails",
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
		return std::string("mercs");
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

	static Mercs NewEntity()
	{
		Mercs e{};

		e.MercID           = 0;
		e.OwnerCharacterID = 0;
		e.Slot             = 0;
		e.Name             = "";
		e.TemplateID       = 0;
		e.SuspendedTime    = 0;
		e.IsSuspended      = 0;
		e.TimerRemaining   = 0;
		e.Gender           = 0;
		e.MercSize         = 5;
		e.StanceID         = 0;
		e.HP               = 0;
		e.Mana             = 0;
		e.Endurance        = 0;
		e.Face             = 1;
		e.LuclinHairStyle  = 1;
		e.LuclinHairColor  = 1;
		e.LuclinEyeColor   = 1;
		e.LuclinEyeColor2  = 1;
		e.LuclinBeardColor = 1;
		e.LuclinBeard      = 0;
		e.DrakkinHeritage  = 0;
		e.DrakkinTattoo    = 0;
		e.DrakkinDetails   = 0;

		return e;
	}

	static Mercs GetMercs(
		const std::vector<Mercs> &mercss,
		int mercs_id
	)
	{
		for (auto &mercs : mercss) {
			if (mercs.MercID == mercs_id) {
				return mercs;
			}
		}

		return NewEntity();
	}

	static Mercs FindOne(
		Database& db,
		int mercs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				mercs_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Mercs e{};

			e.MercID           = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.OwnerCharacterID = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.Slot             = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.Name             = row[3] ? row[3] : "";
			e.TemplateID       = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.SuspendedTime    = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.IsSuspended      = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.TimerRemaining   = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.Gender           = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.MercSize         = row[9] ? strtof(row[9], nullptr) : 5;
			e.StanceID         = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.HP               = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.Mana             = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.Endurance        = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.Face             = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 1;
			e.LuclinHairStyle  = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 1;
			e.LuclinHairColor  = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 1;
			e.LuclinEyeColor   = row[17] ? static_cast<uint32_t>(strtoul(row[17], nullptr, 10)) : 1;
			e.LuclinEyeColor2  = row[18] ? static_cast<uint32_t>(strtoul(row[18], nullptr, 10)) : 1;
			e.LuclinBeardColor = row[19] ? static_cast<uint32_t>(strtoul(row[19], nullptr, 10)) : 1;
			e.LuclinBeard      = row[20] ? static_cast<uint32_t>(strtoul(row[20], nullptr, 10)) : 0;
			e.DrakkinHeritage  = row[21] ? static_cast<uint32_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.DrakkinTattoo    = row[22] ? static_cast<uint32_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.DrakkinDetails   = row[23] ? static_cast<uint32_t>(strtoul(row[23], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int mercs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				mercs_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Mercs &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.OwnerCharacterID));
		v.push_back(columns[2] + " = " + std::to_string(e.Slot));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.Name) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.TemplateID));
		v.push_back(columns[5] + " = " + std::to_string(e.SuspendedTime));
		v.push_back(columns[6] + " = " + std::to_string(e.IsSuspended));
		v.push_back(columns[7] + " = " + std::to_string(e.TimerRemaining));
		v.push_back(columns[8] + " = " + std::to_string(e.Gender));
		v.push_back(columns[9] + " = " + std::to_string(e.MercSize));
		v.push_back(columns[10] + " = " + std::to_string(e.StanceID));
		v.push_back(columns[11] + " = " + std::to_string(e.HP));
		v.push_back(columns[12] + " = " + std::to_string(e.Mana));
		v.push_back(columns[13] + " = " + std::to_string(e.Endurance));
		v.push_back(columns[14] + " = " + std::to_string(e.Face));
		v.push_back(columns[15] + " = " + std::to_string(e.LuclinHairStyle));
		v.push_back(columns[16] + " = " + std::to_string(e.LuclinHairColor));
		v.push_back(columns[17] + " = " + std::to_string(e.LuclinEyeColor));
		v.push_back(columns[18] + " = " + std::to_string(e.LuclinEyeColor2));
		v.push_back(columns[19] + " = " + std::to_string(e.LuclinBeardColor));
		v.push_back(columns[20] + " = " + std::to_string(e.LuclinBeard));
		v.push_back(columns[21] + " = " + std::to_string(e.DrakkinHeritage));
		v.push_back(columns[22] + " = " + std::to_string(e.DrakkinTattoo));
		v.push_back(columns[23] + " = " + std::to_string(e.DrakkinDetails));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.MercID
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Mercs InsertOne(
		Database& db,
		Mercs e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.MercID));
		v.push_back(std::to_string(e.OwnerCharacterID));
		v.push_back(std::to_string(e.Slot));
		v.push_back("'" + Strings::Escape(e.Name) + "'");
		v.push_back(std::to_string(e.TemplateID));
		v.push_back(std::to_string(e.SuspendedTime));
		v.push_back(std::to_string(e.IsSuspended));
		v.push_back(std::to_string(e.TimerRemaining));
		v.push_back(std::to_string(e.Gender));
		v.push_back(std::to_string(e.MercSize));
		v.push_back(std::to_string(e.StanceID));
		v.push_back(std::to_string(e.HP));
		v.push_back(std::to_string(e.Mana));
		v.push_back(std::to_string(e.Endurance));
		v.push_back(std::to_string(e.Face));
		v.push_back(std::to_string(e.LuclinHairStyle));
		v.push_back(std::to_string(e.LuclinHairColor));
		v.push_back(std::to_string(e.LuclinEyeColor));
		v.push_back(std::to_string(e.LuclinEyeColor2));
		v.push_back(std::to_string(e.LuclinBeardColor));
		v.push_back(std::to_string(e.LuclinBeard));
		v.push_back(std::to_string(e.DrakkinHeritage));
		v.push_back(std::to_string(e.DrakkinTattoo));
		v.push_back(std::to_string(e.DrakkinDetails));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.MercID = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Mercs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.MercID));
			v.push_back(std::to_string(e.OwnerCharacterID));
			v.push_back(std::to_string(e.Slot));
			v.push_back("'" + Strings::Escape(e.Name) + "'");
			v.push_back(std::to_string(e.TemplateID));
			v.push_back(std::to_string(e.SuspendedTime));
			v.push_back(std::to_string(e.IsSuspended));
			v.push_back(std::to_string(e.TimerRemaining));
			v.push_back(std::to_string(e.Gender));
			v.push_back(std::to_string(e.MercSize));
			v.push_back(std::to_string(e.StanceID));
			v.push_back(std::to_string(e.HP));
			v.push_back(std::to_string(e.Mana));
			v.push_back(std::to_string(e.Endurance));
			v.push_back(std::to_string(e.Face));
			v.push_back(std::to_string(e.LuclinHairStyle));
			v.push_back(std::to_string(e.LuclinHairColor));
			v.push_back(std::to_string(e.LuclinEyeColor));
			v.push_back(std::to_string(e.LuclinEyeColor2));
			v.push_back(std::to_string(e.LuclinBeardColor));
			v.push_back(std::to_string(e.LuclinBeard));
			v.push_back(std::to_string(e.DrakkinHeritage));
			v.push_back(std::to_string(e.DrakkinTattoo));
			v.push_back(std::to_string(e.DrakkinDetails));

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

	static std::vector<Mercs> All(Database& db)
	{
		std::vector<Mercs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Mercs e{};

			e.MercID           = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.OwnerCharacterID = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.Slot             = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.Name             = row[3] ? row[3] : "";
			e.TemplateID       = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.SuspendedTime    = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.IsSuspended      = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.TimerRemaining   = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.Gender           = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.MercSize         = row[9] ? strtof(row[9], nullptr) : 5;
			e.StanceID         = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.HP               = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.Mana             = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.Endurance        = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.Face             = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 1;
			e.LuclinHairStyle  = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 1;
			e.LuclinHairColor  = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 1;
			e.LuclinEyeColor   = row[17] ? static_cast<uint32_t>(strtoul(row[17], nullptr, 10)) : 1;
			e.LuclinEyeColor2  = row[18] ? static_cast<uint32_t>(strtoul(row[18], nullptr, 10)) : 1;
			e.LuclinBeardColor = row[19] ? static_cast<uint32_t>(strtoul(row[19], nullptr, 10)) : 1;
			e.LuclinBeard      = row[20] ? static_cast<uint32_t>(strtoul(row[20], nullptr, 10)) : 0;
			e.DrakkinHeritage  = row[21] ? static_cast<uint32_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.DrakkinTattoo    = row[22] ? static_cast<uint32_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.DrakkinDetails   = row[23] ? static_cast<uint32_t>(strtoul(row[23], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Mercs> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Mercs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Mercs e{};

			e.MercID           = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.OwnerCharacterID = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.Slot             = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.Name             = row[3] ? row[3] : "";
			e.TemplateID       = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.SuspendedTime    = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.IsSuspended      = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.TimerRemaining   = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.Gender           = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.MercSize         = row[9] ? strtof(row[9], nullptr) : 5;
			e.StanceID         = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.HP               = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.Mana             = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.Endurance        = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.Face             = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 1;
			e.LuclinHairStyle  = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 1;
			e.LuclinHairColor  = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 1;
			e.LuclinEyeColor   = row[17] ? static_cast<uint32_t>(strtoul(row[17], nullptr, 10)) : 1;
			e.LuclinEyeColor2  = row[18] ? static_cast<uint32_t>(strtoul(row[18], nullptr, 10)) : 1;
			e.LuclinBeardColor = row[19] ? static_cast<uint32_t>(strtoul(row[19], nullptr, 10)) : 1;
			e.LuclinBeard      = row[20] ? static_cast<uint32_t>(strtoul(row[20], nullptr, 10)) : 0;
			e.DrakkinHeritage  = row[21] ? static_cast<uint32_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.DrakkinTattoo    = row[22] ? static_cast<uint32_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.DrakkinDetails   = row[23] ? static_cast<uint32_t>(strtoul(row[23], nullptr, 10)) : 0;

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
		const Mercs &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.MercID));
		v.push_back(std::to_string(e.OwnerCharacterID));
		v.push_back(std::to_string(e.Slot));
		v.push_back("'" + Strings::Escape(e.Name) + "'");
		v.push_back(std::to_string(e.TemplateID));
		v.push_back(std::to_string(e.SuspendedTime));
		v.push_back(std::to_string(e.IsSuspended));
		v.push_back(std::to_string(e.TimerRemaining));
		v.push_back(std::to_string(e.Gender));
		v.push_back(std::to_string(e.MercSize));
		v.push_back(std::to_string(e.StanceID));
		v.push_back(std::to_string(e.HP));
		v.push_back(std::to_string(e.Mana));
		v.push_back(std::to_string(e.Endurance));
		v.push_back(std::to_string(e.Face));
		v.push_back(std::to_string(e.LuclinHairStyle));
		v.push_back(std::to_string(e.LuclinHairColor));
		v.push_back(std::to_string(e.LuclinEyeColor));
		v.push_back(std::to_string(e.LuclinEyeColor2));
		v.push_back(std::to_string(e.LuclinBeardColor));
		v.push_back(std::to_string(e.LuclinBeard));
		v.push_back(std::to_string(e.DrakkinHeritage));
		v.push_back(std::to_string(e.DrakkinTattoo));
		v.push_back(std::to_string(e.DrakkinDetails));

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
		const std::vector<Mercs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.MercID));
			v.push_back(std::to_string(e.OwnerCharacterID));
			v.push_back(std::to_string(e.Slot));
			v.push_back("'" + Strings::Escape(e.Name) + "'");
			v.push_back(std::to_string(e.TemplateID));
			v.push_back(std::to_string(e.SuspendedTime));
			v.push_back(std::to_string(e.IsSuspended));
			v.push_back(std::to_string(e.TimerRemaining));
			v.push_back(std::to_string(e.Gender));
			v.push_back(std::to_string(e.MercSize));
			v.push_back(std::to_string(e.StanceID));
			v.push_back(std::to_string(e.HP));
			v.push_back(std::to_string(e.Mana));
			v.push_back(std::to_string(e.Endurance));
			v.push_back(std::to_string(e.Face));
			v.push_back(std::to_string(e.LuclinHairStyle));
			v.push_back(std::to_string(e.LuclinHairColor));
			v.push_back(std::to_string(e.LuclinEyeColor));
			v.push_back(std::to_string(e.LuclinEyeColor2));
			v.push_back(std::to_string(e.LuclinBeardColor));
			v.push_back(std::to_string(e.LuclinBeard));
			v.push_back(std::to_string(e.DrakkinHeritage));
			v.push_back(std::to_string(e.DrakkinTattoo));
			v.push_back(std::to_string(e.DrakkinDetails));

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
