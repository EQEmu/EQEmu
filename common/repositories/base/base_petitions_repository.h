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

class BasePetitionsRepository {
public:
	struct Petitions {
		uint32_t    dib;
		uint32_t    petid;
		std::string charname;
		std::string accountname;
		std::string lastgm;
		std::string petitiontext;
		std::string gmtext;
		std::string zone;
		int32_t     urgency;
		int32_t     charclass;
		int32_t     charrace;
		int32_t     charlevel;
		int32_t     checkouts;
		int32_t     unavailables;
		int8_t      ischeckedout;
		int64_t     senttime;
	};

	static std::string PrimaryKey()
	{
		return std::string("dib");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"dib",
			"petid",
			"charname",
			"accountname",
			"lastgm",
			"petitiontext",
			"gmtext",
			"zone",
			"urgency",
			"charclass",
			"charrace",
			"charlevel",
			"checkouts",
			"unavailables",
			"ischeckedout",
			"senttime",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"dib",
			"petid",
			"charname",
			"accountname",
			"lastgm",
			"petitiontext",
			"gmtext",
			"zone",
			"urgency",
			"charclass",
			"charrace",
			"charlevel",
			"checkouts",
			"unavailables",
			"ischeckedout",
			"senttime",
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
		return std::string("petitions");
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

	static Petitions NewEntity()
	{
		Petitions e{};

		e.dib          = 0;
		e.petid        = 0;
		e.charname     = "";
		e.accountname  = "";
		e.lastgm       = "";
		e.petitiontext = "";
		e.gmtext       = "";
		e.zone         = "";
		e.urgency      = 0;
		e.charclass    = 0;
		e.charrace     = 0;
		e.charlevel    = 0;
		e.checkouts    = 0;
		e.unavailables = 0;
		e.ischeckedout = 0;
		e.senttime     = 0;

		return e;
	}

	static Petitions GetPetitions(
		const std::vector<Petitions> &petitionss,
		int petitions_id
	)
	{
		for (auto &petitions : petitionss) {
			if (petitions.dib == petitions_id) {
				return petitions;
			}
		}

		return NewEntity();
	}

	static Petitions FindOne(
		Database& db,
		int petitions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				petitions_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Petitions e{};

			e.dib          = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.petid        = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.charname     = row[2] ? row[2] : "";
			e.accountname  = row[3] ? row[3] : "";
			e.lastgm       = row[4] ? row[4] : "";
			e.petitiontext = row[5] ? row[5] : "";
			e.gmtext       = row[6] ? row[6] : "";
			e.zone         = row[7] ? row[7] : "";
			e.urgency      = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.charclass    = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.charrace     = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.charlevel    = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.checkouts    = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.unavailables = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.ischeckedout = row[14] ? static_cast<int8_t>(atoi(row[14])) : 0;
			e.senttime     = row[15] ? strtoll(row[15], nullptr, 10) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int petitions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				petitions_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Petitions &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.petid));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.charname) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.accountname) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.lastgm) + "'");
		v.push_back(columns[5] + " = '" + Strings::Escape(e.petitiontext) + "'");
		v.push_back(columns[6] + " = '" + Strings::Escape(e.gmtext) + "'");
		v.push_back(columns[7] + " = '" + Strings::Escape(e.zone) + "'");
		v.push_back(columns[8] + " = " + std::to_string(e.urgency));
		v.push_back(columns[9] + " = " + std::to_string(e.charclass));
		v.push_back(columns[10] + " = " + std::to_string(e.charrace));
		v.push_back(columns[11] + " = " + std::to_string(e.charlevel));
		v.push_back(columns[12] + " = " + std::to_string(e.checkouts));
		v.push_back(columns[13] + " = " + std::to_string(e.unavailables));
		v.push_back(columns[14] + " = " + std::to_string(e.ischeckedout));
		v.push_back(columns[15] + " = " + std::to_string(e.senttime));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.dib
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Petitions InsertOne(
		Database& db,
		Petitions e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.dib));
		v.push_back(std::to_string(e.petid));
		v.push_back("'" + Strings::Escape(e.charname) + "'");
		v.push_back("'" + Strings::Escape(e.accountname) + "'");
		v.push_back("'" + Strings::Escape(e.lastgm) + "'");
		v.push_back("'" + Strings::Escape(e.petitiontext) + "'");
		v.push_back("'" + Strings::Escape(e.gmtext) + "'");
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.urgency));
		v.push_back(std::to_string(e.charclass));
		v.push_back(std::to_string(e.charrace));
		v.push_back(std::to_string(e.charlevel));
		v.push_back(std::to_string(e.checkouts));
		v.push_back(std::to_string(e.unavailables));
		v.push_back(std::to_string(e.ischeckedout));
		v.push_back(std::to_string(e.senttime));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.dib = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Petitions> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.dib));
			v.push_back(std::to_string(e.petid));
			v.push_back("'" + Strings::Escape(e.charname) + "'");
			v.push_back("'" + Strings::Escape(e.accountname) + "'");
			v.push_back("'" + Strings::Escape(e.lastgm) + "'");
			v.push_back("'" + Strings::Escape(e.petitiontext) + "'");
			v.push_back("'" + Strings::Escape(e.gmtext) + "'");
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.urgency));
			v.push_back(std::to_string(e.charclass));
			v.push_back(std::to_string(e.charrace));
			v.push_back(std::to_string(e.charlevel));
			v.push_back(std::to_string(e.checkouts));
			v.push_back(std::to_string(e.unavailables));
			v.push_back(std::to_string(e.ischeckedout));
			v.push_back(std::to_string(e.senttime));

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

	static std::vector<Petitions> All(Database& db)
	{
		std::vector<Petitions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Petitions e{};

			e.dib          = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.petid        = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.charname     = row[2] ? row[2] : "";
			e.accountname  = row[3] ? row[3] : "";
			e.lastgm       = row[4] ? row[4] : "";
			e.petitiontext = row[5] ? row[5] : "";
			e.gmtext       = row[6] ? row[6] : "";
			e.zone         = row[7] ? row[7] : "";
			e.urgency      = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.charclass    = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.charrace     = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.charlevel    = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.checkouts    = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.unavailables = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.ischeckedout = row[14] ? static_cast<int8_t>(atoi(row[14])) : 0;
			e.senttime     = row[15] ? strtoll(row[15], nullptr, 10) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Petitions> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Petitions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Petitions e{};

			e.dib          = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.petid        = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.charname     = row[2] ? row[2] : "";
			e.accountname  = row[3] ? row[3] : "";
			e.lastgm       = row[4] ? row[4] : "";
			e.petitiontext = row[5] ? row[5] : "";
			e.gmtext       = row[6] ? row[6] : "";
			e.zone         = row[7] ? row[7] : "";
			e.urgency      = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.charclass    = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.charrace     = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.charlevel    = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.checkouts    = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.unavailables = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.ischeckedout = row[14] ? static_cast<int8_t>(atoi(row[14])) : 0;
			e.senttime     = row[15] ? strtoll(row[15], nullptr, 10) : 0;

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
		const Petitions &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.dib));
		v.push_back(std::to_string(e.petid));
		v.push_back("'" + Strings::Escape(e.charname) + "'");
		v.push_back("'" + Strings::Escape(e.accountname) + "'");
		v.push_back("'" + Strings::Escape(e.lastgm) + "'");
		v.push_back("'" + Strings::Escape(e.petitiontext) + "'");
		v.push_back("'" + Strings::Escape(e.gmtext) + "'");
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.urgency));
		v.push_back(std::to_string(e.charclass));
		v.push_back(std::to_string(e.charrace));
		v.push_back(std::to_string(e.charlevel));
		v.push_back(std::to_string(e.checkouts));
		v.push_back(std::to_string(e.unavailables));
		v.push_back(std::to_string(e.ischeckedout));
		v.push_back(std::to_string(e.senttime));

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
		const std::vector<Petitions> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.dib));
			v.push_back(std::to_string(e.petid));
			v.push_back("'" + Strings::Escape(e.charname) + "'");
			v.push_back("'" + Strings::Escape(e.accountname) + "'");
			v.push_back("'" + Strings::Escape(e.lastgm) + "'");
			v.push_back("'" + Strings::Escape(e.petitiontext) + "'");
			v.push_back("'" + Strings::Escape(e.gmtext) + "'");
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.urgency));
			v.push_back(std::to_string(e.charclass));
			v.push_back(std::to_string(e.charrace));
			v.push_back(std::to_string(e.charlevel));
			v.push_back(std::to_string(e.checkouts));
			v.push_back(std::to_string(e.unavailables));
			v.push_back(std::to_string(e.ischeckedout));
			v.push_back(std::to_string(e.senttime));

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
