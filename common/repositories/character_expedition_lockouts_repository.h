#pragma once

#include "common/repositories/base/base_character_expedition_lockouts_repository.h"

#include "common/database.h"
#include "common/dynamic_zone_lockout.h"
#include "common/strings.h"
#include "fmt/ranges.h"

#include <unordered_map>

class CharacterExpeditionLockoutsRepository: public BaseCharacterExpeditionLockoutsRepository {
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
     * CharacterExpeditionLockoutsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * CharacterExpeditionLockoutsRepository::GetWhereNeverExpires()
     * CharacterExpeditionLockoutsRepository::GetWhereXAndY()
     * CharacterExpeditionLockoutsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static std::unordered_map<uint32_t, std::vector<DzLockout>> GetLockouts(
		Database& db, const std::vector<uint32_t>& char_ids, const std::string& expedition)
	{
		auto results = db.QueryDatabase(fmt::format(SQL(
			SELECT
				character_id,
				UNIX_TIMESTAMP(expire_time),
				duration,
				event_name,
				from_expedition_uuid
			FROM character_expedition_lockouts
			WHERE
				character_id IN ({0})
				AND expire_time > NOW()
				AND expedition_name = '{1}'
			ORDER BY
				FIELD(character_id, {0}),
				FIELD(event_name, '{2}') DESC
		),
			fmt::join(char_ids, ","),
			Strings::Escape(expedition),
			Strings::Escape(DzLockout::ReplayTimer)
		));

		std::unordered_map<uint32_t, std::vector<DzLockout>> lockouts;

		for (auto row = results.begin(); row != results.end(); ++row)
		{
			int col = 0;
			uint32_t    char_id     = std::strtoul(row[col++], nullptr, 10);
			time_t      expire_time = std::strtoull(row[col++], nullptr, 10);
			uint32_t    duration    = std::strtoul(row[col++], nullptr, 10);
			std::string event       = row[col++];
			std::string uuid        = row[col++];

			lockouts[char_id].emplace_back(std::move(uuid), expedition, std::move(event), expire_time, duration);
		}

		return lockouts;
	}

	static std::vector<DzLockout> GetLockouts(Database& db, uint32_t char_id)
	{
		std::vector<DzLockout> lockouts;

		auto rows = GetWhere(db, fmt::format("character_id = {} AND expire_time > NOW()", char_id));
		lockouts.reserve(rows.size());

		for (auto& row : rows)
		{
			lockouts.emplace_back(
				std::move(row.from_expedition_uuid),
				std::move(row.expedition_name),
				std::move(row.event_name),
				row.expire_time,
				row.duration
			);
		}

		return lockouts;
	}

	static std::vector<CharacterExpeditionLockouts> GetLockouts(Database& db, const std::vector<std::string>& names, const std::string& expedition, const std::string& event)
	{
		if (names.empty())
		{
			return {};
		}

		return GetWhere(db, fmt::format(
			"character_id IN (select id from character_data where name IN ('{}')) AND expire_time > NOW() AND expedition_name = '{}' AND event_name = '{}' LIMIT 1",
			fmt::join(names, "','"), Strings::Escape(expedition), Strings::Escape(event)));
	}

	static void InsertLockouts(Database& db, uint32_t char_id, const std::vector<DzLockout>& lockouts)
	{
		std::string insert_values;
		for (const auto& lockout : lockouts)
		{
			fmt::format_to(std::back_inserter(insert_values),
				"({}, FROM_UNIXTIME({}), {}, '{}', '{}', '{}'),",
				char_id,
				lockout.GetExpireTime(),
				lockout.GetDuration(),
				Strings::Escape(lockout.UUID()),
				Strings::Escape(lockout.DzName()),
				Strings::Escape(lockout.Event())
			);
		}

		if (!insert_values.empty())
		{
			insert_values.pop_back(); // trailing comma

			auto query = fmt::format(SQL(
				INSERT INTO character_expedition_lockouts
					(character_id, expire_time, duration, from_expedition_uuid, expedition_name, event_name)
				VALUES {}
				ON DUPLICATE KEY UPDATE
					from_expedition_uuid = VALUES(from_expedition_uuid),
					expire_time = VALUES(expire_time),
					duration = VALUES(duration);
			), insert_values);

			db.QueryDatabase(query);
		}
	}

	static void InsertLockout(Database& db, const std::vector<uint32_t>& char_ids, const DzLockout& lockout)
	{
		std::string insert_values;
		for (const auto& char_id : char_ids)
		{
			fmt::format_to(std::back_inserter(insert_values),
				"({}, FROM_UNIXTIME({}), {}, '{}', '{}', '{}'),",
				char_id,
				lockout.GetExpireTime(),
				lockout.GetDuration(),
				Strings::Escape(lockout.UUID()),
				Strings::Escape(lockout.DzName()),
				Strings::Escape(lockout.Event())
			);
		}

		if (!insert_values.empty())
		{
			insert_values.pop_back(); // trailing comma

			auto query = fmt::format(SQL(
				INSERT INTO character_expedition_lockouts
					(character_id, expire_time, duration, from_expedition_uuid, expedition_name, event_name)
				VALUES {}
				ON DUPLICATE KEY UPDATE
					from_expedition_uuid = VALUES(from_expedition_uuid),
					expire_time = VALUES(expire_time),
					duration = VALUES(duration);
			), insert_values);

			db.QueryDatabase(query);
		}
	}

	// inserts a new lockout or updates existing lockout with seconds added to current time
	static void AddLockoutDuration(Database& db, const std::vector<uint32_t>& char_ids, const DzLockout& lockout, int seconds)
	{
		std::string insert_values;
		for (const auto& char_id : char_ids)
		{
			fmt::format_to(std::back_inserter(insert_values),
				"({}, FROM_UNIXTIME({}), {}, '{}', '{}', '{}'),",
				char_id,
				lockout.GetExpireTime(),
				lockout.GetDuration(),
				Strings::Escape(lockout.UUID()),
				Strings::Escape(lockout.DzName()),
				Strings::Escape(lockout.Event())
			);
		}

		if (!insert_values.empty())
		{
			insert_values.pop_back(); // trailing comma

			auto query = fmt::format(SQL(
				INSERT INTO character_expedition_lockouts
					(character_id, expire_time, duration, from_expedition_uuid, expedition_name, event_name)
				VALUES {0}
				ON DUPLICATE KEY UPDATE
					from_expedition_uuid = VALUES(from_expedition_uuid),
					expire_time = DATE_ADD(expire_time, INTERVAL {1} SECOND),
					duration = GREATEST(0, CAST(duration AS SIGNED) + {1});
			), insert_values, seconds);

			db.QueryDatabase(query);
		}
	}

};
