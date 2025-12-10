#include "../world_server_cli.h"
#include "../../common/database_schema.h"
#include "../../common/json/json.h"

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

	std::stringstream payload;
	payload << schema;

	std::cout << payload.str() << std::endl;
}
