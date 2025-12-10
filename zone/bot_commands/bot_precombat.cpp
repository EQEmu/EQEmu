#include "../bot_command.h"

void bot_command_precombat(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_precombat", sep->arg[0], "precombat")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([set | clear])", sep->arg[0]);

		return;
	}

	if (!c->GetTarget() || !c->IsAttackAllowed(c->GetTarget())) {
		c->Message(Chat::White, "This command requires an attackable target.");

		return;
	}

	if (RuleB(Bots, BotsRequireLoS) && !c->DoLosChecks(c->GetTarget())) {
		c->Message(Chat::Red, "You must have Line of Sight to use this command.");

		return;
	}

	std::string argument(sep->arg[1]);

	if (!argument.compare("set")) {
		c->SetBotPrecombat(true);
	}
	else if (!argument.compare("clear")) {
		c->SetBotPrecombat(false);
	}
	else {
		c->SetBotPrecombat(!c->GetBotPrecombat());
	}

	c->Message(Chat::White, "Precombat flag is now %s.", (c->GetBotPrecombat() ? "set" : "clear"));
}
