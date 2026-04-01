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

class WorldserverCLI {
public:
	static void CommandHandler(int argc, char **argv);
	static void BotsEnable(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void BotsDisable(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void MercsEnable(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void MercsDisable(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void Version(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void CopyCharacter(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void DatabaseVersion(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void DatabaseSetAccountStatus(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void DatabaseGetSchema(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void DatabaseDump(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void DatabaseUpdates(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestCommand(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestColors(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void ExpansionTestCommand(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestRepository(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestRepository2(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestDatabaseConcurrency(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestStringBenchmarkCommand(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void EtlGetSettings(int argc, char **argv, argh::parser &cmd, std::string &description);
};
