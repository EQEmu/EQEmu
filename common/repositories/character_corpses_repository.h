#pragma once

#include "common/repositories/base/base_character_corpses_repository.h"
#include "common/rulesys.h"

#include "common/database.h"
#include "common/strings.h"

#include "glm/vec4.hpp"

class CharacterCorpsesRepository: public BaseCharacterCorpsesRepository {
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
     * CharacterCorpsesRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * CharacterCorpsesRepository::GetWhereNeverExpires()
     * CharacterCorpsesRepository::GetWhereXAndY()
     * CharacterCorpsesRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static int BuryCorpse(Database& db, uint32 corpse_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `is_buried` = 1 WHERE `{}` = {}",
				TableName(),
				PrimaryKey(),
				corpse_id
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static int BuryDecayedCorpses(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `is_buried` = 1 WHERE `is_buried` = 0 AND (UNIX_TIMESTAMP() - UNIX_TIMESTAMP(time_of_death)) > {} AND time_of_death != 0",
				TableName(),
				RuleI(Character, CorpseDecayTime) / 1000
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static int BuryInstance(Database& db, uint16 instance_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET is_buried = 1, instance_id = 0 WHERE instance_id = {}",
				TableName(),
				instance_id
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static int BuryInstances(Database& db, const std::string& joined_instance_ids)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET is_buried = 1, instance_id = 0 WHERE instance_id IN ({})",
				TableName(),
				joined_instance_ids
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static uint32 GetDecayTimer(Database& db, uint32 corpse_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT (UNIX_TIMESTAMP() - UNIX_TIMESTAMP(time_of_death)) FROM `{}` WHERE `{}` = {} AND `time_of_death` != 0",
				TableName(),
				PrimaryKey(),
				corpse_id
			)
		);

		if (!results.Success() || !results.RowCount()) {
			return 0;
		}

		auto row = results.begin();

		return Strings::ToUnsignedInt(row[0]);
	}

	static uint32 ResurrectCorpse(Database& db, uint32 corpse_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `is_rezzed` = 1 WHERE `{}` = {}",
				TableName(),
				PrimaryKey(),
				corpse_id
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static void SendAdventureCorpsesToGraveyard(
		Database& db,
		uint32 graveyard_zone_id,
		uint16 instance_id,
		const glm::vec4& position
	)
	{
		db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `zone_id` = {}, `instance_id` = 0, `x` = {:.2f}, `y` = {:.2f}, `z` = {:.2f}, `heading` = {:.2f}, `was_at_graveyard` = 1 WHERE `instance_id` = {}",
				TableName(),
				graveyard_zone_id,
				position.x,
				position.y,
				position.z,
				position.w,
				instance_id
			)
		);
	}

	static int SendToGraveyard(
		Database& db,
		uint32 corpse_id,
		uint32 zone_id,
		uint16 instance_id,
		const glm::vec4& position
	)
	{
		db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `zone_id` = {}, `instance_id` = {}, `x` = {:.2f}, `y` = {:.2f}, `z` = {:.2f}, `heading` = {:.2f}, `was_at_graveyard` = 1 WHERE `{}` = {}",
				TableName(),
				zone_id,
				instance_id,
				position.x,
				position.y,
				position.z,
				position.w,
				PrimaryKey(),
				corpse_id
			)
		);

		return corpse_id;
	}

	static void SendToNonInstance(Database& db, uint32 corpse_id)
	{
		db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `instance_id` = 0 WHERE `{}` = {}",
				TableName(),
				PrimaryKey(),
				corpse_id
			)
		);
	}

	static uint32 SetGuildConsentID(Database& db, uint32 character_id, uint32 guild_consent_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `guild_consent_id` = {} WHERE `charid` = {}",
				TableName(),
				guild_consent_id,
				character_id
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static int UnburyCorpse(
		Database& db,
		uint32 corpse_id,
		uint32 zone_id,
		uint16 instance_id,
		const glm::vec4& position
	)
	{
		auto corpse = FindOne(db, corpse_id);

		if (corpse.id == 0) {
			return 0;
		}

		corpse.is_buried		= 0;
		corpse.zone_id			= zone_id;
		corpse.instance_id		= instance_id;
		corpse.x				= position.x;
		corpse.y				= position.y;
		corpse.z				= position.z;
		corpse.heading			= position.w;
		corpse.time_of_death	= time(nullptr);
		corpse.was_at_graveyard = 0;

		return UpdateOne(db, corpse);
	}

	static int UpdateEntityVariables(Database& db, uint32 corpse_id, const std::string& json_string)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `entity_variables` = '{}' WHERE `{}` = {}",
				TableName(),
				Strings::Escape(json_string),
				PrimaryKey(),
				corpse_id
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}
};
