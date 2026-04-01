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

#include "common/repositories/base/base_dynamic_zone_lockouts_repository.h"

#include "common/database.h"
#include "common/dynamic_zone_lockout.h"
#include "common/strings.h"

class DynamicZoneLockoutsRepository: public BaseDynamicZoneLockoutsRepository {
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
     * DynamicZoneLockoutsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * DynamicZoneLockoutsRepository::GetWhereNeverExpires()
     * DynamicZoneLockoutsRepository::GetWhereXAndY()
     * DynamicZoneLockoutsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static void InsertLockouts(Database& db, uint32_t dz_id, const std::vector<DzLockout>& lockouts)
	{
		std::string insert_values;
		for (const auto& lockout : lockouts)
		{
			fmt::format_to(std::back_inserter(insert_values),
				"({}, '{}', '{}', FROM_UNIXTIME({}), {}),",
				dz_id,
				Strings::Escape(lockout.UUID()),
				Strings::Escape(lockout.Event()),
				lockout.GetExpireTime(),
				lockout.GetDuration()
			);
		}

		if (!insert_values.empty())
		{
			insert_values.pop_back(); // trailing comma

			auto query = fmt::format(SQL(
				INSERT INTO dynamic_zone_lockouts
					(dynamic_zone_id, from_expedition_uuid, event_name, expire_time, duration)
				VALUES {}
				ON DUPLICATE KEY UPDATE
					from_expedition_uuid = VALUES(from_expedition_uuid),
					expire_time = VALUES(expire_time),
					duration = VALUES(duration);
			), insert_values);

			db.QueryDatabase(query);
		}
	}

};
