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

#include "common/repositories/base/base_respawn_times_repository.h"

#include "common/database.h"
#include "common/strings.h"

class RespawnTimesRepository: public BaseRespawnTimesRepository {
public:

	static void ClearExpiredRespawnTimers(Database& db)
	{
		db.QueryDatabase(
			fmt::format(
				"DELETE FROM `{}` WHERE `expire_at` < UNIX_TIMESTAMP(NOW())",
				TableName()
			)
		);
	}

	static uint32 GetTimeRemaining(Database& db, uint32 spawn2_id, uint16 instance_id, time_t time_seconds)
	{
		const auto& l = RespawnTimesRepository::GetWhere(
			db,
			fmt::format(
				"`id` = {} AND `instance_id` = {}",
				spawn2_id,
				instance_id
			)
		);

		if (l.empty()) {
			return 0;
		}

		auto r = l.front();

		if ((r.start + r.duration) <= time_seconds) {
			return 0;
		}

		return ((r.start + r.duration) - time_seconds);
	}

	static void ClearInstanceTimers(Database &db, int32_t id)
	{
		RespawnTimesRepository::DeleteWhere(db, fmt::format("`instance_id` = {}", id));
	}

	static int ShiftInstanceTimers(Database& db, uint16 instance_id, uint32_t suspended_at, uint32_t offline_seconds)
	{
		if (instance_id == 0 || suspended_at == 0 || offline_seconds == 0)
		{
			return 0;
		}

		auto results = db.QueryDatabase(fmt::format(
			"UPDATE `{}` "
			"SET `start` = `start` + {}, `expire_at` = `expire_at` + {} "
			"WHERE `instance_id` = {} AND `expire_at` > {}",
			TableName(),
			offline_seconds,
			offline_seconds,
			instance_id,
			suspended_at
		));

		return results.Success() ? results.RowsAffected() : 0;
	}
};
