#pragma once

#include "common/repositories/base/base_fellowship_members_repository.h"

#include "common/database.h"
#include "common/strings.h"

class FellowshipMembersRepository: public BaseFellowshipMembersRepository {
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
     * FellowshipMembersRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * FellowshipMembersRepository::GetWhereNeverExpires()
     * FellowshipMembersRepository::GetWhereXAndY()
     * FellowshipMembersRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	// a member with the character_data fields the roster shows for offline members
	struct MemberWithCharacter {
		FellowshipMembers member;
		bool              is_character_valid; // the character exists and is not soft-deleted
		std::string       name;
		uint32_t          level;
		uint32_t          class_;
		uint32_t          last_login;
	};

	static std::vector<MemberWithCharacter> GetWhereWithCharacters(Database &db, const std::string &where_filter)
	{
		std::vector<MemberWithCharacter> entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT m.`character_id`, m.`fellowship_id`, m.`joined_at`, m.`is_exp_sharing`, "
				"c.`id`, c.`deleted_at`, c.`name`, c.`level`, c.`class`, c.`last_login` "
				"FROM `{}` m LEFT JOIN `character_data` c ON c.`id` = m.`character_id` WHERE {} "
				"ORDER BY m.`fellowship_id`, m.`joined_at`, m.`character_id`",
				TableName(),
				where_filter
			)
		);

		entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MemberWithCharacter e{};

			e.member.character_id   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.member.fellowship_id  = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.member.joined_at      = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.member.is_exp_sharing = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.is_character_valid    = row[4] && !row[5];
			e.name                  = row[6] ? row[6] : "";
			e.level                 = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.class_                = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.last_login            = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;

			entries.push_back(e);
		}

		return entries;
	}

};
