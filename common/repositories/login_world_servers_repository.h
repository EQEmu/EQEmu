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

#include "common/repositories/base/base_login_world_servers_repository.h"

#include "common/database.h"
#include "common/strings.h"

class LoginWorldServersRepository: public BaseLoginWorldServersRepository {
public:
	static LoginWorldServers GetFromWorldContext(Database &db, LoginWorldContext c) {
		std::string where = fmt::format(
			"short_name = '{}' AND long_name = '{}'",
			Strings::Escape(c.short_name),
			Strings::Escape(c.long_name)
		);

		if (c.admin_id > 0) {
			where += fmt::format(" AND login_server_admin_id = {}", c.admin_id);
		}

		where += " LIMIT 1";

		auto results = GetWhere(db, where);
		auto e = NewEntity();
		if (results.size() == 1) {
			e = results.front();
		}

		return e;
	}
};
