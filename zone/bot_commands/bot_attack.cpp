#include "../bot_command.h"

void bot_command_attack(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_attack", sep->arg[0], "attack")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {

		c->Message(Chat::White, "usage: <enemy_target> %s [actionable: byname | ownergroup | ownerraid | namesgroup | healrotation | mmr | byclass | byrace | default: spawned] ([actionable_name])", sep->arg[0]);
		c->Message(Chat::White, "note: Orders bots to attack a designated target");
		return;
	}

	const int ab_mask = ActionableBots::ABM_Type2;
	Mob* target_mob = ActionableTarget::AsSingle_ByAttackable(c);

	if (!target_mob) {

		c->Message(Chat::Yellow, "You must <target> an enemy to use this command");
		return;
	}
	
	if (RuleB(Bots, BotsRequireLoS) && !c->DoLosChecks(target_mob)) {
		c->Message(Chat::Red, "You must have Line of Sight to use this command.");
		return;
	}

	std::string ab_arg(sep->arg[1]);
	if (ab_arg.empty()) {
		ab_arg = "spawned";
	}

	std::string class_race_arg(sep->arg[1]);
	bool class_race_check = false;

	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;

	if (ActionableBots::PopulateSBL(c, ab_arg.c_str(), sbl, ab_mask, !class_race_check ? sep->arg[2] : nullptr, class_race_check ? atoi(sep->arg[2]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	if (!c->HasBotAttackFlag(target_mob)) {
		target_mob->SetBotAttackFlag(c->CharacterID());
		target_mob->bot_attack_flag_timer.Start(10000);
	}

	size_t attacker_count = 0;
	Bot *first_attacker = nullptr;
	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	for (auto bot_iter : sbl) {
		if (!bot_iter->ValidStateCheck(c)) {
			continue;
		}

		if (!first_attacker) {
			first_attacker = bot_iter;
		}

		++attacker_count;

		bot_iter->SetAttackFlag();
	}

	if (first_attacker) {
		std::string message;

		if (attacker_count == 1) {
			message = fmt::format("Attacking {}.", target_mob->GetCleanName());
		} else {
			message = fmt::format("{} of your bots are attacking {}.", sbl.size(), target_mob->GetCleanName());
		}

		c->Message(Chat::Green, message.c_str());
	}
	else {
		c->Message(Chat::Yellow,fmt::format("None of your bots are capable of attacking {}.", target_mob->GetCleanName()).c_str());
	}

	return;
}
