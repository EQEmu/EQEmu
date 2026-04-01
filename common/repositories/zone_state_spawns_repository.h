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
#pragma once

#include "common/repositories/base/base_zone_state_spawns_repository.h"

#include "common/database.h"
#include "common/strings.h"
#include "fmt/ranges.h"

class ZoneStateSpawnsRepository : public BaseZoneStateSpawnsRepository {
public:
	static void PurgeInvalidZoneStates(Database &database)
	{
		std::string query = R"(
			SELECT zone_id, instance_id
			FROM zone_state_spawns
			GROUP BY zone_id, instance_id
			HAVING COUNT(*) = SUM(
				CASE
					WHEN hp = 0
					 AND mana = 0
					 AND endurance = 0
					 AND (loot_data IS NULL OR loot_data = '')
					 AND (entity_variables IS NULL OR entity_variables = '')
					 AND (buffs IS NULL OR buffs = '')
					THEN 1 ELSE 0
				END
			);
		)";

		auto results = database.QueryDatabase(query);
		if (!results.Success()) {
			return;
		}

		for (auto row: results) {
			uint32 zone_id = std::stoul(row[0]);
			uint32 instance_id = std::stoul(row[1]);

			int rows = ZoneStateSpawnsRepository::DeleteWhere(
				database,
				fmt::format(
					"`zone_id` = {} AND `instance_id` = {}",
					zone_id,
					instance_id
				)
			);

			LogInfo(
				"Purged invalid zone state data for zone [{}] instance [{}] rows [{}]",
				zone_id,
				instance_id,
				Strings::Commify(rows)
			);
		}
	}

	static void PurgeOldZoneStates(Database &database)
	{
		int days = RuleI(Zone, StateSaveClearDays);

		std::string query = fmt::format(
			"DELETE FROM zone_state_spawns WHERE created_at < NOW() - INTERVAL {} DAY",
			days
		);

		auto results = database.QueryDatabase(query);
		if (!results.Success()) {
			LogError("Failed to purge old zone state data older than {} days.", days);
			return;
		}

		if (results.RowsAffected() > 0) {
			LogInfo(
				"Purged old zone state data older than days [{}] rows [{}]",
				days,
				Strings::Commify(results.RowsAffected())
			);
		}
	}

};
