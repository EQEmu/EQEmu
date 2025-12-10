#include "../world_server_cli.h"
#include "../worlddb.h"
#include "../../common/database_schema.h"

void WorldserverCLI::BotsDisable(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Disables bots and drops tables";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	std::string input;
	std::cout << "Warning! This will drop all bot tables, are you sure? [y/n]";
	std::getline(std::cin, input);

	if (Strings::ToLower(input) != "y") {
		LogInfo("Aborting");
		return;
	}

	// drop bot tables
	std::string sql = "SET FOREIGN_KEY_CHECKS = 0;";
	for (auto &t: DatabaseSchema::GetBotTables()) {
		sql += fmt::format("DROP TABLE IF EXISTS {};", t);
		LogInfo("Dropping table [{}]", t);
	}

	sql += "SET FOREIGN_KEY_CHECKS = 1;";

	database.QueryDatabaseMulti(sql);

	// disable bots
	LogInfo("Setting rule Bots:Enabled to false");
	RuleManager::Instance()->SetRule("Bots:Enabled", "false", &database, true, true);
	LogInfo("Bots disabled");
}
