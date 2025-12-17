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
};
