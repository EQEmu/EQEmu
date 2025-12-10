#include "../bot_command.h"

void bot_command_surname(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == '\0' || sep->IsNumber(1)) {
		c->Message(Chat::White, "You must specify a [surname] to use this command (use _ to define spaces or -remove to clear.)");
		return;
	}
	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}
	if (strlen(sep->arg[1]) > 31) {
		c->Message(Chat::White, "Surname must be 31 characters or less.");
		return;
	}
	std::string bot_surname = sep->arg[1];
	bot_surname = (bot_surname == "-remove") ? "" : bot_surname;
	std::replace(bot_surname.begin(), bot_surname.end(), '_', ' ');

	my_bot->SetSurname(bot_surname);
	if (database.botdb.SaveBot(my_bot)) {
		c->Message(Chat::White, "Bot Surname Saved.");
	}
}

void bot_command_title(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == '\0' || sep->IsNumber(1)) {
		c->Message(Chat::White, "You must specify a [title] to use this command. (use _ to define spaces or -remove to clear.)");
		return;
	}
	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}
	if (strlen(sep->arg[1]) > 31) {
		c->Message(Chat::White, "Title must be 31 characters or less.");
		return;
	}
	std::string bot_title = sep->arg[1];
	bot_title = (bot_title == "-remove") ? "" : bot_title;
	std::replace(bot_title.begin(), bot_title.end(), '_', ' ');

	my_bot->SetTitle(bot_title);
	if (database.botdb.SaveBot(my_bot)) {
		c->Message(Chat::White, "Bot Title Saved.");
	}
}

void bot_command_suffix(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == '\0' || sep->IsNumber(1)) {
		c->Message(Chat::White, "You must specify a [suffix] to use this command. (use _ to define spaces or -remove to clear.)");
		return;
	}
	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}
	if (strlen(sep->arg[1]) > 31) {
		c->Message(Chat::White, "Suffix must be 31 characters or less.");
		return;
	}
	std::string bot_suffix = sep->arg[1];
	bot_suffix = (bot_suffix == "-remove") ? "" : bot_suffix;
	std::replace(bot_suffix.begin(), bot_suffix.end(), '_', ' ');

	my_bot->SetSuffix(bot_suffix);
	if (database.botdb.SaveBot(my_bot)) {
		c->Message(Chat::White, "Bot Suffix Saved.");
	}
}
