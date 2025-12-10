#include "../bot_command.h"

void bot_command_release(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_release", sep->arg[0], "release"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationmembers | healrotationtargets | mmr | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());
	for (auto bot_iter : sbl) {
		bot_iter->WipeHateList();

		if (bot_iter->GetPet() && bot_iter->HasControllablePet(BotAnimEmpathy::BackOff)) {
			bot_iter->GetPet()->WipeHateList();
			bot_iter->GetPet()->SetTarget(nullptr);
		}

		bot_iter->SetPauseAI(false);
	}

	c->Message(Chat::White, "%i of your bots %s released.", sbl.size(), ((sbl.size() != 1) ? ("are") : ("is")));
}
