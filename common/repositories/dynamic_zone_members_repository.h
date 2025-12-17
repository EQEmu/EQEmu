#pragma once

#include "common/repositories/base/base_dynamic_zone_members_repository.h"

#include "common/database.h"
#include "fmt/ranges.h"

class DynamicZoneMembersRepository: public BaseDynamicZoneMembersRepository {
public:

    /**
     * This file was auto generated and can be modified and extended upon
     *
     * Base repository methods are automatically
     * generated in the "base" version of this repository. The base repository
     * is immutable and to be left untouched, while methods in this class
     * are used as extension methods for more specific persistence-layer
     * accessors or mutators.
     *
     * Base Methods (Subject to be expanded upon in time)
     *
     * Note: Not all tables are designed appropriately to fit functionality with all base methods
     *
     * InsertOne
     * UpdateOne
     * DeleteOne
     * FindOne
     * GetWhere(std::string where_filter)
     * DeleteWhere(std::string where_filter)
     * InsertMany
     * All
     *
     * Example custom methods in a repository
     *
     * DynamicZoneMembersRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * DynamicZoneMembersRepository::GetWhereNeverExpires()
     * DynamicZoneMembersRepository::GetWhereXAndY()
     * DynamicZoneMembersRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	struct MemberWithName {
		uint32_t id;
		uint32_t dynamic_zone_id;
		uint32_t character_id;
		std::string character_name;
	};

	static std::string SelectMembersWithNames()
	{
		return std::string(SQL(
			SELECT
				dynamic_zone_members.id,
				dynamic_zone_members.dynamic_zone_id,
				dynamic_zone_members.character_id,
				character_data.name
			FROM dynamic_zone_members
				INNER JOIN character_data ON dynamic_zone_members.character_id = character_data.id
		));
	}

	static std::vector<MemberWithName> AllWithNames(Database& db)
	{
		std::vector<MemberWithName> all_entries;

		auto results = db.QueryDatabase(SelectMembersWithNames());
		if (results.Success())
		{
			all_entries.reserve(results.RowCount());

			for (auto row = results.begin(); row != results.end(); ++row)
			{
				MemberWithName entry{};

				int col = 0;
				entry.id                = strtoul(row[col++], nullptr, 10);
				entry.dynamic_zone_id   = strtoul(row[col++], nullptr, 10);
				entry.character_id      = strtoul(row[col++], nullptr, 10);
				entry.character_name    = row[col++];

				all_entries.emplace_back(std::move(entry));
			}
		}

		return all_entries;
	}

	static int DeleteByInstance(Database& db, int instance_id)
	{
		auto results = db.QueryDatabase(fmt::format(SQL(
			DELETE dynamic_zone_members
			FROM dynamic_zone_members
				INNER JOIN dynamic_zones ON dynamic_zone_members.dynamic_zone_id = dynamic_zones.id
			WHERE dynamic_zones.instance_id = {}
		), instance_id));

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int DeleteByManyInstances(Database& db, const std::string& joined_instance_ids)
	{
		auto results = db.QueryDatabase(fmt::format(SQL(
			DELETE dynamic_zone_members
			FROM dynamic_zone_members
				INNER JOIN dynamic_zones ON dynamic_zone_members.dynamic_zone_id = dynamic_zones.id
			WHERE dynamic_zones.instance_id IN ({})
		), joined_instance_ids));

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int GetCountWhere(Database& db, const std::string& where_filter)
	{
		auto results = db.QueryDatabase(fmt::format(
			"SELECT COUNT(*) FROM {} WHERE {};", TableName(), where_filter));

		uint32_t count = 0;
		if (results.Success() && results.RowCount() > 0)
		{
			auto row = results.begin();
			count = strtoul(row[0], nullptr, 10);
		}
		return count;
	}

	static void AddMember(Database& db, uint32_t dynamic_zone_id, uint32_t character_id)
	{
		db.QueryDatabase(fmt::format(SQL(
			INSERT INTO {}
				(dynamic_zone_id, character_id)
			VALUES
				({}, {})
			ON DUPLICATE KEY UPDATE id = id;
		),
			TableName(),
			dynamic_zone_id,
			character_id
		));
	}

	static void RemoveMember(Database& db, uint32_t dynamic_zone_id, uint32_t character_id)
	{
		DeleteWhere(db, fmt::format("dynamic_zone_id = {} AND character_id = {}", dynamic_zone_id, character_id));
	}

	static void RemoveAllMembers(Database& db, uint32_t dynamic_zone_id)
	{
		DeleteWhere(db, fmt::format("dynamic_zone_id = {}", dynamic_zone_id));
	}

	static uint32_t InsertOrUpdateMany(Database& db, const std::vector<DynamicZoneMembers>& entries)
	{
		if (entries.empty())
		{
			return 0;
		}

		std::vector<std::string> values;
		values.reserve(entries.size());

		for (const auto& entry : entries)
		{
			values.push_back(fmt::format("({},{},{})", entry.id, entry.dynamic_zone_id, entry.character_id));
		}

		auto results = db.QueryDatabase(fmt::format(
			"INSERT INTO {} ({}) VALUES {} ON DUPLICATE KEY UPDATE id = id;",
			TableName(), ColumnsRaw(), fmt::join(values, ",")));

		return results.Success() ? results.RowsAffected() : 0;
	}
};
