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

#include "common/cli/eqemu_command_handler.h"

namespace LoginserverCommandHandler {
	void CommandHandler(int argc, char **argv);
	void CreateLoginserverApiToken(int argc, char **argv, argh::parser &cmd, std::string &description);
	void ListLoginserverApiTokens(int argc, char **argv, argh::parser &cmd, std::string &description);
	void CreateLocalLoginserverAccount(int argc, char **argv, argh::parser &cmd, std::string &description);
	void CreateLoginserverWorldAdminAccount(int argc, char **argv, argh::parser &cmd, std::string &description);
	void CheckLoginserverUserCredentials(int argc, char **argv, argh::parser &cmd, std::string &description);
	void UpdateLoginserverUserCredentials(int argc, char **argv, argh::parser &cmd, std::string &description);
	void CheckExternalLoginserverUserCredentials(int argc, char **argv, argh::parser &cmd, std::string &description);
	void UpdateLoginserverWorldAdminAccountPassword(int argc, char **argv, argh::parser &cmd, std::string &description);
	void HealthCheckLogin(int argc, char **argv, argh::parser &cmd, std::string &description);
};
