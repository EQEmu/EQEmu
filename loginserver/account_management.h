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

#include "common/types.h"
#include "loginserver/login_types.h"
#include "loginserver/login_server.h"

#include <string>

extern LoginServer server;
extern Database database;

class AccountManagement {
public:
	static uint64 CreateLoginServerAccount(LoginAccountContext c);
	static uint64 CheckLoginserverUserCredentials(LoginAccountContext c);
	static bool UpdateLoginserverUserCredentials(LoginAccountContext c);
	static uint64 CheckExternalLoginserverUserCredentials(LoginAccountContext c);
	static bool UpdateLoginserverWorldAdminAccountPasswordByName(LoginAccountContext c);
	static uint64 HealthCheckUserLogin();

	static bool CreateLoginserverWorldAdminAccount(
		const std::string &username,
		const std::string &password,
		const std::string &email,
		const std::string &first_name = "",
		const std::string &last_name = "",
		const std::string &ip_address = ""
	);
};
