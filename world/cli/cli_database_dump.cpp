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
#include "world/world_server_cli.h"

#include "common/database/database_dump_service.h"

void WorldserverCLI::DatabaseDump(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Dumps server database tables";

	std::vector<std::string> arguments = {};
	std::vector<std::string> options   = {
		"--all",
		"--content-tables",
		"--login-tables",
		"--player-tables",
		"--bot-tables",
		"--merc-tables",
		"--state-tables",
		"--system-tables",
		"--static-instance-data",
		"--table-structure-only",
		"--table-lock",
		"--dump-path=",
		"--dump-output-to-console",
		"--drop-table-syntax-only",
		"--compress"
	};

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

	auto s        = new DatabaseDumpService();
	bool dump_all = cmd[{"-a", "--all"}];

	if (!cmd("--dump-path").str().empty()) {
		s->SetDumpPath(cmd("--dump-path").str());
	}

	// options
	s->SetDumpContentTables(cmd[{"--content-tables"}] || dump_all);
	s->SetDumpLoginServerTables(cmd[{"--login-tables"}] || dump_all);
	s->SetDumpPlayerTables(cmd[{"--player-tables"}] || dump_all);
	s->SetDumpBotTables(cmd[{"--bot-tables"}] || dump_all);
	s->SetDumpMercTables(cmd[{"--merc-tables"}] || dump_all);
	s->SetDumpStateTables(cmd[{"--state-tables"}] || dump_all);
	s->SetDumpSystemTables(cmd[{"--system-tables"}] || dump_all);
	s->SetDumpAllTables(dump_all);
	s->SetDumpWithNoData(cmd[{"--table-structure-only"}]);
	s->SetDumpTableLock(cmd[{"--table-lock"}]);
	s->SetDumpWithCompression(cmd[{"--compress"}]);
	s->SetDumpOutputToConsole(cmd[{"--dump-output-to-console"}]);
	s->SetDumpDropTableSyntaxOnly(cmd[{"--drop-table-syntax-only"}]);
	s->SetDumpStaticInstanceData(cmd[{"--static-instance-data"}]);

	s->DatabaseDump();
}
