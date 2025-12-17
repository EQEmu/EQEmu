#pragma once

#include "common/repositories/base/base_bot_inspect_messages_repository.h"

#include "common/database.h"
#include "common/strings.h"

class BotInspectMessagesRepository: public BaseBotInspectMessagesRepository {
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
     * BotInspectMessagesRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * BotInspectMessagesRepository::GetWhereNeverExpires()
     * BotInspectMessagesRepository::GetWhereXAndY()
     * BotInspectMessagesRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static bool SaveAllInspectMessages(Database& db, const uint32 owner_id, const std::string& inspect_message)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				SQL(
					INSERT INTO `bot_inspect_messages` (`bot_id`, `inspect_message`) VALUES
					(SELECT `bot_id`, '{}' inspect_message FROM `bot_data` WHERE `owner_id` = {})
				),
				inspect_message,
				owner_id
			)
		);

		return results.Success();
	}

	static bool DeleteAllInspectMessages(Database& db, const uint32 owner_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				SQL(
					DELETE FROM `bot_inspect_messages`
					WHERE `bot_id` IN (SELECT `bot_id` FROM `bot_data` WHERE `owner_id` = {})
				),
				owner_id
			)
		);

		return results.Success();
	}
};
