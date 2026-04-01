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
