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
#include "world_server_cli.h"

void WorldserverCLI::CommandHandler(int argc, char **argv)
{
	if (argc == 1) { return; }

	argh::parser cmd;
	cmd.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
	EQEmuCommand::DisplayDebug(cmd);

	// Declare command mapping
	auto function_map = EQEmuCommand::function_map;

	// Register commands
	function_map["bots:enable"]                 = &WorldserverCLI::BotsEnable;
	function_map["bots:disable"]                = &WorldserverCLI::BotsDisable;
	function_map["mercs:enable"]                = &WorldserverCLI::MercsEnable;
	function_map["mercs:disable"]               = &WorldserverCLI::MercsDisable;
	function_map["world:version"]               = &WorldserverCLI::Version;
	function_map["character:copy-character"]    = &WorldserverCLI::CopyCharacter;
	function_map["database:version"]            = &WorldserverCLI::DatabaseVersion;
	function_map["database:set-account-status"] = &WorldserverCLI::DatabaseSetAccountStatus;
	function_map["database:schema"]             = &WorldserverCLI::DatabaseGetSchema;
	function_map["database:dump"]               = &WorldserverCLI::DatabaseDump;
	function_map["database:updates"]            = &WorldserverCLI::DatabaseUpdates;
	function_map["test:test"]                   = &WorldserverCLI::TestCommand;
	function_map["test:colors"]                 = &WorldserverCLI::TestColors;
	function_map["test:expansion"]              = &WorldserverCLI::ExpansionTestCommand;
	function_map["test:repository"]             = &WorldserverCLI::TestRepository;
	function_map["test:repository2"]            = &WorldserverCLI::TestRepository2;
	function_map["test:db-concurrency"]         = &WorldserverCLI::TestDatabaseConcurrency;
	function_map["test:string-benchmark"]       = &WorldserverCLI::TestStringBenchmarkCommand;
	function_map["etl:settings"]                = &WorldserverCLI::EtlGetSettings;

	EQEmuCommand::HandleMenu(function_map, cmd, argc, argv);
}
