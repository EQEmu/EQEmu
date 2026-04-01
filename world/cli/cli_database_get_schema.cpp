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

#include "common/database_schema.h"
#include "common/json/json.h"

#include <iostream>

void WorldserverCLI::DatabaseGetSchema(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Displays server database schema";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	Json::Value              player_tables_json;
	std::vector<std::string> player_tables = DatabaseSchema::GetPlayerTables();
	for (const auto          &table: player_tables) {
		player_tables_json.append(table);
	}

	Json::Value              content_tables_json;
	std::vector<std::string> content_tables = DatabaseSchema::GetContentTables();
	for (const auto          &table: content_tables) {
		content_tables_json.append(table);
	}

	Json::Value              server_tables_json;
	std::vector<std::string> server_tables = DatabaseSchema::GetServerTables();
	for (const auto          &table: server_tables) {
		server_tables_json.append(table);
	}

	Json::Value              login_tables_json;
	std::vector<std::string> login_tables = DatabaseSchema::GetLoginTables();
	for (const auto          &table: login_tables) {
		login_tables_json.append(table);
	}

	Json::Value              state_tables_json;
	std::vector<std::string> state_tables = DatabaseSchema::GetStateTables();
	for (const auto          &table: state_tables) {
		state_tables_json.append(table);
	}

	Json::Value              version_tables_json;
	std::vector<std::string> version_tables = DatabaseSchema::GetVersionTables();
	for (const auto          &table: version_tables) {
		version_tables_json.append(table);
	}

	Json::Value              bot_tables_json;
	std::vector<std::string> bot_tables = DatabaseSchema::GetBotTables();
	for (const auto          &table: bot_tables) {
		bot_tables_json.append(table);
	}

	Json::Value schema;

	schema["content_tables"] = content_tables_json;
	schema["login_tables"]   = login_tables_json;
	schema["player_tables"]  = player_tables_json;
	schema["server_tables"]  = server_tables_json;
	schema["state_tables"]   = state_tables_json;
	schema["version_tables"] = version_tables_json;
	schema["bot_tables"]     = bot_tables_json;

	std::cout << schema << std::endl;
}
