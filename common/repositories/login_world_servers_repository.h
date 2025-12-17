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
