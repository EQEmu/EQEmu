#include "../bot_command.h"

void bot_command_cast(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_cast", sep->arg[0], "cast")) {
		c->Message(Chat::White, "note: Commands bots to force cast a specific spell type, ignoring all settings (holds, delays, thresholds, etc).");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Commands bots to force cast a specific spell type, ignoring all settings (holds, delays, thresholds, etc)." };
		p.notes =
		{
			"- This will interrupt any spell currently being cast by bots told to use the command",
			"- Bots will still check to see if they have the spell in their spell list, whether the target is immune, spell is allowed and all other sanity checks for spells",
			fmt::format(
				"- You can use {} aa # to cast any clickable AA or specifically {} harmtouch / {} layonhands"
				, sep->arg[0]
				, sep->arg[0]
				, sep->arg[0]
			)
		};
		p.example_format =
		{
			fmt::format("{} [Type Shortname] [actionable, default: spawned]", sep->arg[0]),
			fmt::format("{} [Type ID] [actionable, default: spawned]", sep->arg[0])
		};
		p.examples_one =
		{
			"To tell everyone to Nuke the target:",
			fmt::format(
				"{} {}",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::Nuke)
			),
			fmt::format(
				"{} {}",
				sep->arg[0],
				BotSpellTypes::Nuke
			)
		};
		p.examples_two =
		{
			"To tell Skbot to Harm Touch the target:",
			fmt::format(
				"{} aa 6000 byname Skbot",
				sep->arg[0]
			),
			fmt::format(
				"{} harmtouch byname Skbot",
				sep->arg[0]
			)
		};
		p.examples_three =
		{
			"To tell all bots to try to cast spell #93 (Burst of Flame)",
			fmt::format(
				"{} spellid 93",
				sep->arg[0]
			)
		};
		p.actionables = { "target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned" };


		std::string popup_text = c->SendBotCommandHelpWindow(p);
		popup_text = DialogueWindow::Table(popup_text);

		c->SendPopupToClient(sep->arg[0], popup_text.c_str());
		c->SendSpellTypePrompts(true);

		c->Message(
			Chat::Yellow,
			fmt::format(
				"Use help after any command type for more subtypes to use, for example: {}.",
				Saylink::Silent("^cast invisibility help")
			).c_str()
		);

		if (RuleB(Bots, SendClassRaceOnHelp)) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"Use {} for information about race/class IDs.",
					Saylink::Silent("^classracelist")
				).c_str()
			);
		}

		return;
	}

	std::string arg1 = sep->arg[1];
	std::string arg2 = sep->arg[2];

	//AA help
	if (!arg1.compare("aa") && !arg2.compare("help")) {
		c->Message(Chat::Yellow, "Enter the ID of an AA to attempt to cast.", sep->arg[0]);
	}

	//Commanded type help prompts
	if (!arg2.compare("help")) {
		c->Message(Chat::Yellow, "You can also use [single], [group], [ae]. Ex: ^cast movementspeed group.", sep->arg[0]);
	}

	if (!arg1.compare("invisibility") && !arg2.compare("help")) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Available options for {} are: {}, {}, {}, {}.",
				sep->arg[0],
				Saylink::Silent("^cast invisibility see", "see"),
				Saylink::Silent("^cast invisibility invis", "invis"),
				Saylink::Silent("^cast invisibility undead", "undead"),
				Saylink::Silent("^cast invisibility animals", "animals")
			).c_str()
		);

		return;
	}

	if (!arg1.compare("size") && !arg2.compare("help")) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Available options for {} are: {}, {}.",
				sep->arg[0],
				Saylink::Silent("^cast size grow", "grow"),
				Saylink::Silent("^cast size shrink", "shrink")
			).c_str()
		);

		return;
	}

	if (!arg1.compare("movementspeed") && !arg2.compare("help")) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Available options for {} are: {}, {}.",
				sep->arg[0],
				Saylink::Silent("^cast movementspeed selo"), "selo"
			).c_str()
		);

		return;
	}

	if (!arg2.compare("help")) {
		c->Message(Chat::Yellow, "There are no additional options for {}.", sep->arg[0]);
		return;
	}

	int ab_arg = 2;
	uint16 spell_type = UINT16_MAX;
	uint16 sub_type = UINT16_MAX;
	uint16 sub_target_type = UINT16_MAX;
	bool aa_type = false;
	int aa_id = 0;
	bool by_spell_id = false;
	uint16 chosen_spell_id = UINT16_MAX;

	if (!arg1.compare("aa") || !arg1.compare("harmtouch") || !arg1.compare("layonhands")) {
		if (!RuleB(Bots, AllowCastAAs)) {
			c->Message(Chat::Yellow, "This commanded type is currently disabled.");
			return;
		}

		if (!arg1.compare("harmtouch")) {
			aa_id = zone->GetAlternateAdvancementAbilityByRank(aaHarmTouch)->id;
		}
		else if (!arg1.compare("layonhands")) {
			aa_id = zone->GetAlternateAdvancementAbilityByRank(aaLayonHands)->id;
		}
		else if (!sep->IsNumber(2) || !zone->GetAlternateAdvancementAbility(Strings::ToInt(arg2))) {
			c->Message(Chat::Yellow, "You must enter an AA ID.");
			return;
		}
		else {
			++ab_arg;
			aa_id = Strings::ToInt(arg2);
		}

		aa_type = true;		
	}

	if (!arg1.compare("spellid")) {
		if (!RuleB(Bots, AllowForcedCastsBySpellID)) {
			c->Message(Chat::Yellow, "This commanded type is currently disabled.");
			return;
		}

		if (sep->IsNumber(2) && IsValidSpell(atoi(sep->arg[2]))) {
			++ab_arg;
			chosen_spell_id = atoi(sep->arg[2]);
			by_spell_id = true;
		}
		else {
			c->Message(Chat::Yellow, "You must enter a valid spell ID.");

			return;
		}
	}

	if (!aa_type && !by_spell_id) {
		if (sep->IsNumber(1)) {
			spell_type = atoi(sep->arg[1]);

			if (!Bot::IsValidBotSpellType(spell_type)) {
				c->Message(
					Chat::Yellow,
					fmt::format(
						"You must choose a valid spell type. Use {} for information regarding this command.",
						Saylink::Silent(
							fmt::format("{} help", sep->arg[0])
						)
					).c_str()
				);

				return;
			}
		}
		else {
			if (Bot::GetSpellTypeIDByShortName(arg1) != UINT16_MAX) {
				spell_type = Bot::GetSpellTypeIDByShortName(arg1);
			}
			else {
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Incorrect argument, use {} for information regarding this command.",
						Saylink::Silent(
							fmt::format("{} help", sep->arg[0])
						)
					).c_str()
				);

				return;
			}
		}

		switch (spell_type) { //Allowed command checks
			case BotSpellTypes::Charm:
				if (!RuleB(Bots, AllowCommandedCharm)) {
					c->Message(Chat::Yellow, "This commanded type is currently disabled.");
					return;
				}

				break;
			case BotSpellTypes::AEMez:
			case BotSpellTypes::Mez:
				if (!RuleB(Bots, AllowCommandedMez)) {
					c->Message(Chat::Yellow, "This commanded type is currently disabled.");
					return;
				}

				break;
			case BotSpellTypes::Resurrect:
				if (!RuleB(Bots, AllowCommandedResurrect)) {
					c->Message(Chat::Yellow, "This commanded type is currently disabled.");
					return;
				}

				break;
			case BotSpellTypes::AELull:
			case BotSpellTypes::Lull:
				if (!RuleB(Bots, AllowCommandedLull)) {
					c->Message(Chat::Yellow, "This commanded type is currently disabled.");
					return;
				}

				break;
			case BotSpellTypes::SummonCorpse:
				if (!RuleB(Bots, AllowCommandedSummonCorpse)) {
					c->Message(Chat::Yellow, "This commanded type is currently disabled.");
					return;
				}

				break;
			default:
				break;
		}

		std::string arg_string = sep->arg[ab_arg];


		if (!arg_string.compare("shrink")) {
			sub_type = CommandedSubTypes::Shrink;
			++ab_arg;
		}
		else if (!arg_string.compare("grow")) {
			sub_type = CommandedSubTypes::Grow;
			++ab_arg;
		}
		else if (!arg_string.compare("see")) {
			sub_type = CommandedSubTypes::SeeInvis;
			++ab_arg;
		}
		else if (!arg_string.compare("invis")) {
			sub_type = CommandedSubTypes::Invis;
			++ab_arg;
		}
		else if (!arg_string.compare("undead")) {
			sub_type = CommandedSubTypes::InvisUndead;
			++ab_arg;
		}
		else if (!arg_string.compare("animals")) {
			sub_type = CommandedSubTypes::InvisAnimals;
			++ab_arg;
		}
		else if (!arg_string.compare("selo")) {
			sub_type = CommandedSubTypes::Selo;
			++ab_arg;
		}

		arg_string = sep->arg[ab_arg];

		if (!arg_string.compare("single")) {
			sub_target_type = CommandedSubTypes::SingleTarget;
			++ab_arg;
		}
		else if (!arg_string.compare("group")) {
			sub_target_type = CommandedSubTypes::GroupTarget;
			++ab_arg;
		}
		else if (!arg_string.compare("ae")) {
			sub_target_type = CommandedSubTypes::AETarget;
			++ab_arg;
		}
	}

	Mob* tar = c->GetTarget();

	if (!tar) {
		if ((!aa_type && !by_spell_id) && spell_type != BotSpellTypes::Escape && spell_type != BotSpellTypes::Pet) {
			c->Message(Chat::Yellow, "You need a target for that.");
			return;
		}
	}

	if (!aa_type && !by_spell_id) {
		if (IsPetBotSpellType(spell_type) && !tar->IsPet()) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"[{}] is an invalid target. {} requires a pet to be targeted.",
					tar->GetCleanName(),
					Bot::GetSpellTypeNameByID(spell_type)
				).c_str()
			);

			return;
		}

		switch (spell_type) { //Target Checks
			case BotSpellTypes::Resurrect:
				if (!tar->IsCorpse() || !tar->CastToCorpse()->IsPlayerCorpse()) {
					c->Message(
						Chat::Yellow, 
						fmt::format(
							"[{}] is not a player's corpse.", 
							tar->GetCleanName()
						).c_str()
					);

					return;
				}

				break;
			case BotSpellTypes::Identify:
			case BotSpellTypes::SendHome:
			case BotSpellTypes::BindAffinity:
			case BotSpellTypes::SummonCorpse:
				if (!tar->IsClient() || !c->IsInGroupOrRaid(tar)) {
					c->Message(
						Chat::Yellow,
						fmt::format(
							"[{}] is an invalid target. Only players in your group or raid are eligible targets.", 
							tar->GetCleanName()
						).c_str()
					);

					return;
				}

				break;
			default:
				if (
					(IsBotSpellTypeDetrimental(spell_type) && !c->IsAttackAllowed(tar)) ||
					(
						spell_type == BotSpellTypes::Charm &&
						(
							tar->IsClient() ||
							tar->IsCorpse() ||
							tar->GetOwner()
						)
					)
				) {
					c->Message(
						Chat::Yellow,
						fmt::format(
							"You cannot attack [{}].", 
							tar->GetCleanName()
						).c_str()
					);

					return;
				}

				if (IsBotSpellTypeBeneficial(spell_type)) {
					if (
						(tar->IsNPC() && !tar->GetOwner()) ||
						(tar->GetOwner() && tar->GetOwner()->IsOfClientBot() && !c->IsInGroupOrRaid(tar->GetOwner())) ||
						(tar->IsOfClientBot() && !c->IsInGroupOrRaid(tar))
					) {
						c->Message(
							Chat::Yellow,
							fmt::format(
								"[{}] is an invalid target. Only players or their pet in your group or raid are eligible targets.",
								tar->GetCleanName()
							).c_str()
						);

						return;
					}
				}

				break;
		}
	}

	if (
		(spell_type == BotSpellTypes::Cure || spell_type == BotSpellTypes::GroupCures || spell_type == BotSpellTypes::PetCures) &&
		!c->CastToBot()->GetNeedsCured(tar)
	) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"[{}] doesn't have anything that needs to be cured.",
				tar->GetCleanName()
			).c_str()
		);

		return;
	}

	const int ab_mask = ActionableBots::ABM_Type1;
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

	BotSpell bot_spell;
	bot_spell.SpellId = 0;
	bot_spell.SpellIndex = 0;
	bot_spell.ManaCost = 0;
	bool is_success = false;
	uint16 success_count = 0;
	Bot* first_found = nullptr;

	for (auto bot_iter : sbl) {
		if (!bot_iter->ValidStateCheck(c)) {
			continue;
		}

		Mob* new_tar = tar;

		if (!aa_type && !by_spell_id) {
			if (!BotSpellTypeRequiresTarget(spell_type)) {
				new_tar = bot_iter;
			}

			if (!new_tar) {
				continue;
			}

			if (
				IsBotSpellTypeBeneficial(spell_type) &&
				!RuleB(Bots, CrossRaidBuffingAndHealing) &&
				!bot_iter->IsInGroupOrRaid(new_tar, true)
			) {
				continue;
			}

			if (IsBotSpellTypeDetrimental(spell_type) && !bot_iter->IsAttackAllowed(new_tar)) {
				bot_iter->RaidGroupSay(
					fmt::format(
						"I cannot attack [{}].",
						new_tar->GetCleanName()
					).c_str()
				);

				continue;
			}
		}

		if (aa_type) {
			if (!bot_iter->GetAA(zone->GetAlternateAdvancementAbility(aa_id)->first_rank_id)) {
				continue;
			}

			AA::Rank* temp_rank = nullptr;
			AA::Rank*& rank = temp_rank;
			uint16 spell_id = bot_iter->GetSpellByAA(aa_id, rank);

			if (!IsValidSpell(spell_id)) {
				continue;
			}

			bool requires_los = !(IsAnyHealSpell(spell_id) && !IsPBAESpell(spell_id));
			bot_iter->SetHasLoS(requires_los ? bot_iter->DoLosChecks(new_tar) : true);

			if (!bot_iter->AttemptAACastSpell(tar, spell_id, rank)) {
				continue;
			}

			is_success = true;
			++success_count;

			continue;
		}
		else if (by_spell_id) {
			if (!bot_iter->CanUseBotSpell(chosen_spell_id)) {
				continue;
			}

			if (!tar || (spells[chosen_spell_id].target_type == ST_Self && tar != bot_iter)) {
				tar = bot_iter;
			}

			bool los_required = bot_iter != tar && !IsAnyHealSpell(chosen_spell_id) && !IsPBAESpell(chosen_spell_id);
			bot_iter->SetHasLoS(los_required ? bot_iter->DoLosChecks(new_tar) : true);

			if (bot_iter->AttemptForcedCastSpell(tar, chosen_spell_id)) {
				if (!first_found) {
					first_found = bot_iter;
				}

				is_success = true;
				++success_count;
			}

			continue;
		}
		else {
			bot_iter->SetCommandedSpell(true);
			bot_iter->SetHasLoS(BotSpellTypeRequiresLoS(spell_type) ? bot_iter->DoLosChecks(new_tar) : true);

			if (bot_iter->AICastSpell(new_tar, 100, spell_type, sub_target_type, sub_type)) {
				if (!first_found) {
					first_found = bot_iter;
				}

				is_success = true;
				++success_count;
			}

			bot_iter->SetCommandedSpell(false);

			continue;
		}

		continue;
	}

	std::string type = "";

	if (aa_type) {
		type = zone->GetAAName(zone->GetAlternateAdvancementAbility(aa_id)->first_rank_id);
	}
	else if (by_spell_id) {
		type = "Forced";
	}
	else {
		if (sub_type == UINT16_MAX) {
			type = Bot::GetSpellTypeNameByID(spell_type);
		}
		else {
			type = Bot::GetSubTypeNameByID(sub_type);
		}
	}

	if (!is_success) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"No bots are capable of casting [{}] on {}. This could be due to this to any number of things: range, mana, immune, target type, etc.",
				(by_spell_id ? spells[chosen_spell_id].name : type),
				tar ? tar->GetCleanName() : "your target"
			).c_str()
		);

		if (!aa_type && !by_spell_id) {
			helper_send_usage_required_bots(c, spell_type);
		}
	}
	else {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"{} {} [{}]{}",
				((success_count == 1 && first_found) ? first_found->GetCleanName() : (fmt::format("{}", success_count).c_str())),
				((success_count == 1 && first_found) ? "casted" : "of your bots casted"),
				(by_spell_id ? spells[chosen_spell_id].name : type),
				tar ? (fmt::format(" on {}.", tar->GetCleanName()).c_str()) : "."
			).c_str()
		);
	}
}
