#include "../bot_command.h"

void bot_command_pull(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_pull", sep->arg[0], "pull")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <enemy_target> %s ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationtargets | mmr | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}

	const int ab_mask = ActionableBots::ABM_Type1;

	std::string arg1 = sep->arg[1];
	int ab_arg = 1;
	std::string actionable_arg = sep->arg[ab_arg];

	if (actionable_arg.empty()) {
		actionable_arg = "spawned";
	}

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;

	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;

	if (ActionableBots::PopulateSBL(c, actionable_arg, sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	auto target_mob = c->GetTarget();

	if (
		!target_mob ||
		target_mob->IsOfClientBotMerc() ||
		!c->IsAttackAllowed(target_mob)
	) {
		c->Message(Chat::White, "Your current target is not attackable!");

		return;
	}

	if (RuleB(Bots, BotsRequireLoS) && !c->DoLosChecks(target_mob)) {
		c->Message(Chat::Red, "You must have Line of Sight to use this command.");

		return;
	}

	if (target_mob->IsNPC() && target_mob->GetHateList().size()) {
		c->Message(Chat::White, "Your current target is already engaged!");

		return;
	}

	Bot* bot_puller = nullptr;
	Bot* backup_bot_puller = nullptr;
	Bot* alternate_bot_puller = nullptr;
	bool backup_puller_found = false;
	bool alternate_puller_found = false;

	for (auto bot_iter : sbl) {
		if (!bot_iter->ValidStateCheck(c)) {
			continue;
		}

		switch (bot_iter->GetClass()) {
			case Class::Rogue:
			case Class::Monk:
			case Class::Bard:
			case Class::Ranger:
				bot_iter->SetPullFlag();

				return;
			default:
				break;
		}

		if (!backup_puller_found) {
			switch (bot_iter->GetClass()) {
				case Class::Warrior:
				case Class::ShadowKnight:
				case Class::Paladin:
				case Class::Berserker:
				case Class::Beastlord:
					backup_bot_puller = bot_iter;
					backup_puller_found = true;

					break;
				default:
					break;
			}
		}

		if (!backup_puller_found && !alternate_puller_found) {
			alternate_bot_puller = bot_iter;
			alternate_puller_found = true;
		}
	}

	bot_puller = backup_bot_puller ? backup_bot_puller : alternate_bot_puller;

	if (bot_puller) {
		bot_puller->SetPullFlag();
	}

	helper_no_available_bots(c, bot_puller);
}
