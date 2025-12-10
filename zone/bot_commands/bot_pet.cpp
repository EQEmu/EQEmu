#include "../bot_command.h"
#include "../bot.h"

void bot_command_pet(Client *c, const Seperator *sep)
{
	std::vector<const char*> subcommand_list = {
		"petgetlost",
		"petremove",
		"petsettype"
	};

	if (helper_command_alias_fail(c, "bot_command_pet", sep->arg[0], "pet"))
		return;

	helper_send_available_subcommands(c, "bot pet", subcommand_list);
}

void bot_command_pet_get_lost(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_pet_get_lost", sep->arg[0], "petgetlost"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationmembers | healrotationtargets | mmr | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	int ab_mask = ActionableBots::ABM_NoFilter;

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	int summoned_pet = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter->GetPet() || bot_iter->GetPet()->IsCharmed())
			continue;

		bot_iter->GetPet()->SayString(PET_GETLOST_STRING);
		bot_iter->GetPet()->Depop(false);
		bot_iter->SetPetID(0);
		database.botdb.DeletePetItems(bot_iter->GetBotID());
		database.botdb.DeletePetBuffs(bot_iter->GetBotID());
		database.botdb.DeletePetStats(bot_iter->GetBotID());
		++summoned_pet;
	}

	c->Message(Chat::White, "%i of your bots released their summoned pet%s", summoned_pet, (summoned_pet == 1) ? "" : "s");
}

void bot_command_pet_remove(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_pet_remove", sep->arg[0], "petremove"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		return;
	}
	int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	uint16 class_mask = (player_class_bitmasks[Class::Druid] | player_class_bitmasks[Class::Necromancer] | player_class_bitmasks[Class::Enchanter]);
	ActionableBots::Filter_ByClasses(c, sbl, class_mask);
	if (sbl.empty()) {
		c->Message(Chat::White, "You have no spawned bots capable of charming");
		return;
	}
	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	int charmed_pet = 0;
	int summoned_pet = 0;
	for (auto bot_iter : sbl) { // Probably needs some work to release charmed pets
		if (bot_iter->IsBotCharmer()) {
			bot_iter->SetBotCharmer(false);
			if (sbl.size() == 1)
				bot_iter->RaidGroupSay("Using a summoned pet");
			++summoned_pet;
			continue;
		}

		if (bot_iter->GetPet()) {
			bot_iter->GetPet()->SayString(PET_GETLOST_STRING);
			bot_iter->GetPet()->Depop(false);
			bot_iter->SetPetID(0);
		}
		bot_iter->SetBotCharmer(true);
		if (sbl.size() == 1)
			bot_iter->RaidGroupSay("Available for Charming");
		++charmed_pet;
	}

	if (sbl.size() != 1)
		c->Message(Chat::White, "%i of your bots set for charming, %i of your bots set for summoned pet use", charmed_pet, summoned_pet);
}

void bot_command_pet_set_type(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_pet_set_type", sep->arg[0], "petsettype")) {
		c->Message(Chat::White, "note: Allows you to change the type of pet Magician bots will cast.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Allows you to change the type of pet Magician bots will cast." };
		p.notes = {
			fmt::format(
				"- Epic pets are currently {} on this server.",
				RuleB(Bots, AllowMagicianEpicPet) ? "allowed" : "not allowed"
			)
		};
		p.example_format = { fmt::format("{} [current | water | fire | air | earth | monster | epic] [actionable, default: target]", sep->arg[0]) };
		p.examples_one =
		{
			"To set all spawned bots to use Water pets:",
			fmt::format(
				"{} fire spawned",
				sep->arg[0]
			)
		};
		p.examples_two =
		{
			"To set Magelulz to use Fire pets:",
			fmt::format(
				"{} fire byname Magelulz",
				sep->arg[0]
			)
		};
		p.examples_three =
		{
			"To check the current pet type for all bots:",
			fmt::format(
				"{} current spawned",
				sep->arg[0]
			)
		};
		p.actionables = { "target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned" };

		std::string popup_text = c->SendBotCommandHelpWindow(p);
		popup_text = DialogueWindow::Table(popup_text);

		c->SendPopupToClient(sep->arg[0], popup_text.c_str());

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
	int ab_arg = 2;
	bool current_check = false;
	uint8 pet_type = 255;
	uint8 level_req = 255;

	if (!arg1.compare("auto")) {
		pet_type = 0;
	}
	else if (!arg1.compare("water")) {
		pet_type = 1;
	}
	else if (!arg1.compare("fire")) {
		pet_type = 2;
	}
	else if (!arg1.compare("air")) {
		pet_type = 3;
	}
	else if (!arg1.compare("earth")) {
		pet_type = 4;
	}
	else if (!arg1.compare("monster")) {
		pet_type = 5;
	}
	else if (!arg1.compare("epic")) {
		if (!RuleB(Bots, AllowMagicianEpicPet)) {
			c->Message(Chat::Yellow, "Epic pets are currently disabled for bots.");
			return;
		}

		pet_type = 6;
	}
	else if (!arg1.compare("current")) {
		current_check = true;
	}

	if (!current_check && pet_type == 255) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"You must specify a pet [type: auto | water | fire | air | earth | monster | epic]. Use {} for information regarding this command.",
				Saylink::Silent(
					fmt::format("{} help", sep->arg[0])
				)
			).c_str()
		);
		return;
	}

	const int ab_mask = ActionableBots::ABM_Type1;
	std::string actionable_arg = sep->arg[ab_arg];

	if (actionable_arg.empty()) {
		actionable_arg = "target";
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

	std::string current_type;
	uint16 reclaim_energy_id = RuleI(Bots, ReclaimEnergySpellID);
	bool is_success = false;
	uint16 success_count = 0;
	Bot* first_found = nullptr;

	for (auto bot_iter : sbl) {
		if (bot_iter->GetClass() != Class::Magician) {
			continue;
		}

		if (!bot_iter->IsInGroupOrRaid(c)) {
			continue;
		}

		if (pet_type == 6 && RuleI(Bots, RequiredMagicianEpicPetItemID) > 0) {
			bool has_item = bot_iter->HasBotItem(RuleI(Bots, RequiredMagicianEpicPetItemID)) != INVALID_INDEX;

			if (!has_item) {
				c->Message(
					Chat::Say,
					fmt::format(
						"{} says, 'I require {} to cast an epic pet which I do not currently possess.'",
						bot_iter->GetCleanName(),
						(database.GetItem(RuleI(Bots, RequiredMagicianEpicPetItemID)) ? database.CreateItemLink(RuleI(Bots, RequiredMagicianEpicPetItemID)) : "an item")
					).c_str()
				);

				continue;
			}
		}

		if (current_check) {
			switch (bot_iter->GetPetChooserID()) {
				case 0:
					current_type = "auto";
					break;
				case SumWater:
					current_type = "water";
					break;
				case SumFire:
					current_type = "fire";
					break;
				case SumAir:
					current_type = "air";
					break;
				case SumEarth:
					current_type = "earth";
					break;
				case MonsterSum:
					current_type = "monster";
					break;
				case SumMageMultiElement:
					current_type = "epic";
					break;
				default:
					current_type = "null";
					break;
			}

			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'I'm currently summoning {} pets.'",
					bot_iter->GetCleanName(),
					current_type
				).c_str()
			);

			continue;
		}

		uint8 air_min_level = 255;
		uint8 fire_min_level = 255;
		uint8 water_min_level = 255;
		uint8 earth_min_level = 255;
		uint8 monster_min_level = 255;
		uint8 epic_min_level = 255;
		std::list<BotSpell> bot_spell_list = bot_iter->GetBotSpellsBySpellType(bot_iter, BotSpellTypes::Pet);

		for (const auto& s : bot_spell_list) {
			if (!IsValidSpell(s.SpellId)) {
				continue;
			}

			if (!IsEffectInSpell(s.SpellId, SpellEffect::SummonPet)) {
				continue;
			}

			auto spell = spells[s.SpellId];

			if (!strncmp(spell.teleport_zone, "SumWater", 8) && spell.classes[Class::Magician - 1] < water_min_level) {
				water_min_level = spell.classes[Class::Magician - 1];
			}
			else if (!strncmp(spell.teleport_zone, "SumFire", 7) && spell.classes[Class::Magician - 1] < fire_min_level) {
				fire_min_level = spell.classes[Class::Magician - 1];
			}
			else if (!strncmp(spell.teleport_zone, "SumAir", 6) && spell.classes[Class::Magician - 1] < air_min_level) {
				air_min_level = spell.classes[Class::Magician - 1];
			}
			else if (!strncmp(spell.teleport_zone, "SumEarth", 8) && spell.classes[Class::Magician - 1] < earth_min_level) {
				earth_min_level = spell.classes[Class::Magician - 1];
			}
			else if (!strncmp(spell.teleport_zone, "MonsterSum", 10) && spell.classes[Class::Magician - 1] < monster_min_level) {
				monster_min_level = spell.classes[Class::Magician - 1];
			}
		}

		uint8 min_level = std::min({
			water_min_level,
			fire_min_level,
			air_min_level,
			earth_min_level,
			monster_min_level
		});

		epic_min_level = RuleI(Bots, AllowMagicianEpicPetLevel);

		switch (pet_type) {
			case 0:
				level_req = min_level;
				break;
			case SumWater:
				level_req = water_min_level;
				break;
			case SumFire:
				level_req = fire_min_level;
				break;
			case SumAir:
				level_req = air_min_level;
				break;
			case SumEarth:
				level_req = earth_min_level;
				break;
			case MonsterSum:
				level_req = monster_min_level;
				break;
			case SumMageMultiElement:
				level_req = epic_min_level;
				break;
			default:
				break;
		}

		if (bot_iter->GetLevel() < level_req) {
			continue;
		}

		bot_iter->SetPetChooserID(pet_type);

		if (bot_iter->GetPet()) {
			auto pet_id = bot_iter->GetPetID();
			bot_iter->SetPetID(0);
			bot_iter->CastSpell(reclaim_energy_id, pet_id);
		}

		if (!first_found) {
			first_found = bot_iter;
		}

		is_success = true;
		++success_count;
	}

	if (current_check) {
		return;
	}

	if (!is_success) {
		c->Message(Chat::Yellow, "No bots were selected.");

		return;
	}

	if (success_count == 1 && first_found) {
		c->Message(
			Chat::Green,
			fmt::format(
				"{} says, 'I will now summon {} pets.'",
				first_found->GetCleanName(),
				current_type
			).c_str()
		);
	}
	else {
		c->Message(
			Chat::Green,
			fmt::format(
				"{} of your bots will now summon {} pets.",
				success_count,
				arg1
			).c_str()
		);
	}
}
