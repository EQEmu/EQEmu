#include "../world_server_cli.h"
#include "../worlddb.h"
#include "../../common/rulesys.h"

void WorldserverCLI::BotsEnable(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Bootstraps bot tables and enables bots";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	// bootstrap bot tables if they don't exist
	if (!database.DoesTableExist("bot_data")) {
		LogInfo("Bootstrapping bot tables");
		database.SourceSqlFromUrl(
			"https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/bot_tables_bootstrap.sql"
		);
	}
	else {
		LogInfo("Bot tables already exist, skipping bootstrap");
	}

	LogInfo("Enabling bots");
	LogInfo("Setting rule Bots:Enabled to true");
	RuleManager::Instance()->SetRule("Bots:Enabled", "true", &database, true, true);
}
