/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
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

class BaseFellowshipCampsitesRepository {
public:
	struct FellowshipCampsites {
		uint32_t fellowship_id;
		uint16_t zone_id;
		uint16_t instance_id;
		float    x;
		float    y;
		float    z;
		float    heading;
		uint32_t campfire_type;
		uint32_t placed_at;
		uint32_t expires_at;
	};

	static std::string PrimaryKey()
	{
		return std::string("fellowship_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"fellowship_id",
			"zone_id",
			"instance_id",
			"x",
			"y",
			"z",
			"heading",
			"campfire_type",
			"placed_at",
			"expires_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"fellowship_id",
			"zone_id",
			"instance_id",
			"x",
			"y",
			"z",
			"heading",
			"campfire_type",
			"placed_at",
			"expires_at",
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
		return std::string("fellowship_campsites");
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

	static FellowshipCampsites NewEntity()
	{
		FellowshipCampsites e{};

		e.fellowship_id = 0;
		e.zone_id       = 0;
		e.instance_id   = 0;
		e.x             = 0;
		e.y             = 0;
		e.z             = 0;
		e.heading       = 0;
		e.campfire_type = 0;
		e.placed_at     = 0;
		e.expires_at    = 0;

		return e;
	}

	static FellowshipCampsites GetFellowshipCampsites(
		const std::vector<FellowshipCampsites> &fellowship_campsitess,
		int fellowship_campsites_id
	)
	{
		for (auto &fellowship_campsites : fellowship_campsitess) {
			if (fellowship_campsites.fellowship_id == fellowship_campsites_id) {
				return fellowship_campsites;
			}
		}

		return NewEntity();
	}

	static FellowshipCampsites FindOne(
		Database& db,
		int fellowship_campsites_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				fellowship_campsites_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			FellowshipCampsites e{};

			e.fellowship_id = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zone_id       = row[1] ? static_cast<uint16_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.instance_id   = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.x             = row[3] ? strtof(row[3], nullptr) : 0;
			e.y             = row[4] ? strtof(row[4], nullptr) : 0;
			e.z             = row[5] ? strtof(row[5], nullptr) : 0;
			e.heading       = row[6] ? strtof(row[6], nullptr) : 0;
			e.campfire_type = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.placed_at     = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.expires_at    = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int fellowship_campsites_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				fellowship_campsites_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const FellowshipCampsites &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.fellowship_id));
		v.push_back(columns[1] + " = " + std::to_string(e.zone_id));
		v.push_back(columns[2] + " = " + std::to_string(e.instance_id));
		v.push_back(columns[3] + " = " + std::to_string(e.x));
		v.push_back(columns[4] + " = " + std::to_string(e.y));
		v.push_back(columns[5] + " = " + std::to_string(e.z));
		v.push_back(columns[6] + " = " + std::to_string(e.heading));
		v.push_back(columns[7] + " = " + std::to_string(e.campfire_type));
		v.push_back(columns[8] + " = " + std::to_string(e.placed_at));
		v.push_back(columns[9] + " = " + std::to_string(e.expires_at));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.fellowship_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static FellowshipCampsites InsertOne(
		Database& db,
		FellowshipCampsites e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.fellowship_id));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.instance_id));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.campfire_type));
		v.push_back(std::to_string(e.placed_at));
		v.push_back(std::to_string(e.expires_at));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.fellowship_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<FellowshipCampsites> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.fellowship_id));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.instance_id));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.campfire_type));
			v.push_back(std::to_string(e.placed_at));
			v.push_back(std::to_string(e.expires_at));

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

	static std::vector<FellowshipCampsites> All(Database& db)
	{
		std::vector<FellowshipCampsites> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FellowshipCampsites e{};

			e.fellowship_id = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zone_id       = row[1] ? static_cast<uint16_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.instance_id   = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.x             = row[3] ? strtof(row[3], nullptr) : 0;
			e.y             = row[4] ? strtof(row[4], nullptr) : 0;
			e.z             = row[5] ? strtof(row[5], nullptr) : 0;
			e.heading       = row[6] ? strtof(row[6], nullptr) : 0;
			e.campfire_type = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.placed_at     = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.expires_at    = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<FellowshipCampsites> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<FellowshipCampsites> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FellowshipCampsites e{};

			e.fellowship_id = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zone_id       = row[1] ? static_cast<uint16_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.instance_id   = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.x             = row[3] ? strtof(row[3], nullptr) : 0;
			e.y             = row[4] ? strtof(row[4], nullptr) : 0;
			e.z             = row[5] ? strtof(row[5], nullptr) : 0;
			e.heading       = row[6] ? strtof(row[6], nullptr) : 0;
			e.campfire_type = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.placed_at     = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.expires_at    = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;

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
		const FellowshipCampsites &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.fellowship_id));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.instance_id));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.campfire_type));
		v.push_back(std::to_string(e.placed_at));
		v.push_back(std::to_string(e.expires_at));

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
		const std::vector<FellowshipCampsites> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.fellowship_id));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.instance_id));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.campfire_type));
			v.push_back(std::to_string(e.placed_at));
			v.push_back(std::to_string(e.expires_at));

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
