#include "common/server_reload_types.h"
#include "common/strings.h"
#include "zone/client.h"
#include "zone/worldserver.h"

#include <string>

extern WorldServer worldserver;

void command_reload(Client *c, const Seperator *sep)
{
	std::string command   = sep->arg[0] ? sep->arg[0] : "";
	const auto  arguments = sep->argnum;
	if (!arguments && Strings::Contains(command, "#reload")) {
		c->SendReloadCommandMessages();
		return;
	}

	std::string full_command = command;
	for (int    i            = 1; i <= sep->argnum; ++i) {
		full_command += " " + std::string(sep->arg[i]);
	}

	bool is_logs_reload_alias    = sep->arg[0] && Strings::Contains(command, "#rl");
	bool is_opcodes_reload_alias = sep->arg[0] && Strings::Contains(command, "#opcode");
	bool is_rq_alias             = sep->arg[0] && Strings::Contains(command, "#rq");
	if (is_logs_reload_alias) {
		full_command = "#reload logs";
	}
	else if (is_opcodes_reload_alias) {
		full_command = "#reload opcodes";
	}
	else if (is_rq_alias) {
		full_command = "#reload quest";
	}

	auto args = Strings::Split(full_command, ' ');

	bool found_command = false;

	for (auto &t: ServerReload::GetTypes()) {
		std::string reload_slug = Strings::Slugify(ServerReload::GetNameClean(t), "_");
		std::string command_arg = args.size() > 1 ? args[1] : "";
		bool        is_global   = args.size() > 2 && args[2] == "global";
		if (sep->arg[0] && Strings::EqualFold(command_arg, reload_slug)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Attempting to reload [{}] {} from command [{}]",
					ServerReload::GetName(t),
					is_global ? "globally" : "locally",
					full_command
				).c_str()
			);
			worldserver.SendReload(t, is_global);
			found_command = true;
		}
	}

	if (!found_command) {
		c->SendReloadCommandMessages();
	}
}
