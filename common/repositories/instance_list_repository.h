#pragma once

#include "common/repositories/base/base_instance_list_repository.h"

#include "common/database.h"
#include "common/strings.h"

class InstanceListRepository: public BaseInstanceListRepository {
public:
	static int UpdateDuration(Database& db, uint16 instance_id, uint32_t new_duration)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `duration` = {}, `expire_at` = (`duration` + `start_time`) WHERE `{}` = {}",
				TableName(),
				new_duration,
				PrimaryKey(),
				instance_id
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static uint32 GetRemainingTimeByInstanceID(Database& db, uint16 instance_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				SQL(
					SELECT (`expire_at` - UNIX_TIMESTAMP()) AS `remaining` FROM `{}`
					WHERE `id` = {}
				),
				TableName(),
				instance_id
			)
		);

		if (!results.Success() || !results.RowCount()) {
			return 0;
		}

		auto row = results.begin();

		return Strings::ToUnsignedInt(row[0]);
	}
};
