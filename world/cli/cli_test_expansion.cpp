#include "../world_server_cli.h"
#include "../../common/rulesys.h"
#include "../../common/repositories/content_flags_repository.h"
#include "../../common/content/world_content_service.h"
#include "../../common/repositories/criteria/content_filter_criteria.h"
#include "../worlddb.h"

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
