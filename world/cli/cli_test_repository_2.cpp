#include "../world_server_cli.h"
#include "../worlddb.h"
#include "../../common/repositories/zone_repository.h"

void WorldserverCLI::TestRepository2(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Test command";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	auto zones = ZoneRepository::GetWhere(content_db, "short_name = 'anguish'");

	for (auto &zone: zones) {
		LogInfo(
			"Zone [{}] long_name [{}] id [{}]",
			zone.short_name,
			zone.long_name,
			zone.id
		);
	}
}
