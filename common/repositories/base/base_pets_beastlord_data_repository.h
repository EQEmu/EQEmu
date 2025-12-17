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

class BasePetsBeastlordDataRepository {
public:
	struct PetsBeastlordData {
		uint32_t player_race;
		uint32_t pet_race;
		uint8_t  texture;
		uint8_t  helm_texture;
		uint8_t  gender;
		float    size_modifier;
		uint8_t  face;
	};

	static std::string PrimaryKey()
	{
		return std::string("player_race");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"player_race",
			"pet_race",
			"texture",
			"helm_texture",
			"gender",
			"size_modifier",
			"face",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"player_race",
			"pet_race",
			"texture",
			"helm_texture",
			"gender",
			"size_modifier",
			"face",
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
		return std::string("pets_beastlord_data");
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

	static PetsBeastlordData NewEntity()
	{
		PetsBeastlordData e{};

		e.player_race   = 1;
		e.pet_race      = 42;
		e.texture       = 0;
		e.helm_texture  = 0;
		e.gender        = 2;
		e.size_modifier = 1;
		e.face          = 0;

		return e;
	}

	static PetsBeastlordData GetPetsBeastlordData(
		const std::vector<PetsBeastlordData> &pets_beastlord_datas,
		int pets_beastlord_data_id
	)
	{
		for (auto &pets_beastlord_data : pets_beastlord_datas) {
			if (pets_beastlord_data.player_race == pets_beastlord_data_id) {
				return pets_beastlord_data;
			}
		}

		return NewEntity();
	}

	static PetsBeastlordData FindOne(
		Database& db,
		int pets_beastlord_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				pets_beastlord_data_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PetsBeastlordData e{};

			e.player_race   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 1;
			e.pet_race      = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 42;
			e.texture       = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.helm_texture  = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.gender        = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 2;
			e.size_modifier = row[5] ? (strtof(row[5], nullptr) > 0.0f ? strtof(row[5], nullptr) : 1) : 1;
			e.face          = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int pets_beastlord_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				pets_beastlord_data_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const PetsBeastlordData &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.player_race));
		v.push_back(columns[1] + " = " + std::to_string(e.pet_race));
		v.push_back(columns[2] + " = " + std::to_string(e.texture));
		v.push_back(columns[3] + " = " + std::to_string(e.helm_texture));
		v.push_back(columns[4] + " = " + std::to_string(e.gender));
		v.push_back(columns[5] + " = " + std::to_string(e.size_modifier));
		v.push_back(columns[6] + " = " + std::to_string(e.face));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.player_race
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static PetsBeastlordData InsertOne(
		Database& db,
		PetsBeastlordData e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.player_race));
		v.push_back(std::to_string(e.pet_race));
		v.push_back(std::to_string(e.texture));
		v.push_back(std::to_string(e.helm_texture));
		v.push_back(std::to_string(e.gender));
		v.push_back(std::to_string(e.size_modifier));
		v.push_back(std::to_string(e.face));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.player_race = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<PetsBeastlordData> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.player_race));
			v.push_back(std::to_string(e.pet_race));
			v.push_back(std::to_string(e.texture));
			v.push_back(std::to_string(e.helm_texture));
			v.push_back(std::to_string(e.gender));
			v.push_back(std::to_string(e.size_modifier));
			v.push_back(std::to_string(e.face));

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

	static std::vector<PetsBeastlordData> All(Database& db)
	{
		std::vector<PetsBeastlordData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PetsBeastlordData e{};

			e.player_race   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 1;
			e.pet_race      = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 42;
			e.texture       = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.helm_texture  = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.gender        = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 2;
			e.size_modifier = row[5] ? (strtof(row[5], nullptr) > 0.0f ? strtof(row[5], nullptr) : 1) : 1;
			e.face          = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<PetsBeastlordData> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<PetsBeastlordData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PetsBeastlordData e{};

			e.player_race   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 1;
			e.pet_race      = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 42;
			e.texture       = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.helm_texture  = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.gender        = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 2;
			e.size_modifier = row[5] ? (strtof(row[5], nullptr) > 0.0f ? strtof(row[5], nullptr) : 1) : 1;
			e.face          = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;

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
		const PetsBeastlordData &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.player_race));
		v.push_back(std::to_string(e.pet_race));
		v.push_back(std::to_string(e.texture));
		v.push_back(std::to_string(e.helm_texture));
		v.push_back(std::to_string(e.gender));
		v.push_back(std::to_string(e.size_modifier));
		v.push_back(std::to_string(e.face));

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
		const std::vector<PetsBeastlordData> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.player_race));
			v.push_back(std::to_string(e.pet_race));
			v.push_back(std::to_string(e.texture));
			v.push_back(std::to_string(e.helm_texture));
			v.push_back(std::to_string(e.gender));
			v.push_back(std::to_string(e.size_modifier));
			v.push_back(std::to_string(e.face));

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
