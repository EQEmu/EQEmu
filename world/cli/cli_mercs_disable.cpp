#include "world/world_server_cli.h"

#include "common/database_schema.h"
#include "world/worlddb.h"

void WorldserverCLI::MercsDisable(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Disables mercenaries";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	std::string input;
	std::cout << "Warning! This will drop all merc tables, are you sure? [y/n]";
	std::getline(std::cin, input);

	if (Strings::ToLower(input) != "y") {
		LogInfo("Aborting");
		return;
	}

	std::string sql = "SET FOREIGN_KEY_CHECKS = 0;";
	for (auto &t: DatabaseSchema::GetMercTables()) {
		sql += fmt::format("DROP TABLE IF EXISTS {};", t);
		LogInfo("Dropping table [{}]", t);
	}

	sql += "SET FOREIGN_KEY_CHECKS = 1;";

	database.QueryDatabaseMulti(sql);
	LogInfo("Setting rule Mercs:AllowMercs to false");
	RuleManager::Instance()->SetRule("Mercs:AllowMercs", "false", &database, true, true);
	LogInfo("Disabled mercs and dropped tables");
}
