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

#include "common/content/world_content_service.h"
#include "common/repositories/content_flags_repository.h"
#include "common/repositories/criteria/content_filter_criteria.h"
#include "common/rulesys.h"
#include "world/worlddb.h"

void WorldserverCLI::ExpansionTestCommand(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Expansion test command";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	if (!RuleManager::Instance()->LoadRules(&database, "default", false)) {
		LogInfo("No rule set configured, using default rules");
	}

	WorldContentService::Instance()->SetCurrentExpansion(RuleI(Expansion, CurrentExpansion));

	std::vector<ContentFlagsRepository::ContentFlags> flags = {};
	auto                                              f     = ContentFlagsRepository::NewEntity();
	f.enabled = 1;

	std::vector<std::string> flag_names = {
		"hateplane_enabled",
		"patch_nerf_7077",
	};

	for (auto &name: flag_names) {
		f.flag_name = name;
		flags.push_back(f);
	}

	WorldContentService::Instance()->SetContentFlags(flags);

	LogInfo(
		"Current expansion is [{}] ({}) is Velious Enabled [{}] Criteria [{}]",
		WorldContentService::Instance()->GetCurrentExpansion(),
		WorldContentService::Instance()->GetCurrentExpansionName(),
		WorldContentService::Instance()->IsTheScarsOfVeliousEnabled() ? "true" : "false",
		ContentFilterCriteria::apply()
	);
}
