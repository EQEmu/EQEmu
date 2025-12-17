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
