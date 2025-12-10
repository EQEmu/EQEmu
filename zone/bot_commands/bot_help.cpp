#include "../bot_command.h"

void bot_command_help(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_help", sep->arg[0], "help"))
		return;

	c->Message(Chat::White, "Available EQEMu bot commands:");

	int bot_commands_shown = 0;
	for (auto command_iter : bot_command_list) {
		if (sep->arg[1][0] && command_iter.first.find(sep->arg[1]) == std::string::npos)
			continue;
		if (c->Admin() < command_iter.second->access)
			continue;

		c->Message(
			Chat::White,
			fmt::format(
				"^{} - {}",
				command_iter.first,
				command_iter.second->desc ? command_iter.second->desc : "No Description"
			).c_str()
		);

		++bot_commands_shown;
	}
	if (parse->PlayerHasQuestSub(EVENT_BOT_COMMAND)) {
		int i = parse->EventPlayer(EVENT_BOT_COMMAND, c, sep->msg, 0);
		if (i >= 1) {
			bot_commands_shown += i;
		}
	}
	c->Message(Chat::White, "%d bot command%s listed.", bot_commands_shown, bot_commands_shown != 1 ? "s" : "");
	c->Message(Chat::White, "type %ccommand [help | usage] for more information", BOT_COMMAND_CHAR);
}
