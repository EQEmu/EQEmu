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

#include "common/repositories/base/base_login_server_admins_repository.h"

#include "common/database.h"
#include "common/strings.h"

class LoginServerAdminsRepository : public BaseLoginServerAdminsRepository {
public:
	static LoginServerAdmins GetByName(Database &db, std::string account_name)
	{
		auto admins = GetWhere(
			db,
			fmt::format(
				"account_name = '{}' LIMIT 1",
				Strings::Escape(account_name)
			)
		);

		if (admins.size() == 1) {
			return admins.front();
		}

		return NewEntity();
	}
};
