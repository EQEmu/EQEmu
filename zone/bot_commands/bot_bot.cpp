#include "../bot_command.h"

void bot_command_bot(Client *c, const Seperator *sep)
{
	std::vector<const char*> subcommand_list = {
		"botappearance",
		"botcamp",
		"botclone",
		"botcreate",
		"botdelete",
		"botfollowdistance",
		"botinspectmessage",
		"botlist",
		"botoutofcombat",
		"botreport",
		"botspawn",
		"botstance",
		"botstopmeleelevel",
		"botsummon",
		"bottoggleranged",
		"bottogglehelm",
		"botupdate"
	};

	if (helper_command_alias_fail(c, "bot_command_bot", sep->arg[0], "bot"))
		return;

	helper_send_available_subcommands(c, "bot", subcommand_list);
}

void bot_command_camp(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_camp", sep->arg[0], "botcamp")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationtargets | mmr | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}

	if (!Bot::CheckCampSpawnConditions(c)) {
		return;
	}

	const int ab_mask = ActionableBots::ABM_Type1;

	std::string class_race_arg = sep->arg[1];
	bool class_race_check = false;
	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, !class_race_check ? sep->arg[2] : nullptr, class_race_check ? atoi(sep->arg[2]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	for (auto bot_iter : sbl) {
		bot_iter->Camp();
	}

	if (!sbl.empty()) {
		c->Message(Chat::White, fmt::format("{} of your bots have been camped.", sbl.size()).c_str());
	}
}

void bot_command_clone(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_clone", sep->arg[0], "botclone")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: <target_bot> {} [clone_name]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must target a bot that you own to use this command!");
		return;
	}

	if (!my_bot->GetBotID()) {
		c->Message(
			Chat::White,
			fmt::format(
				"An unknown error has occured with {} (Bot ID {}).",
				my_bot->GetCleanName(),
				my_bot->GetBotID()
			).c_str()
		);
		LogCommands(
			"bot_command_clone(): - Error: Active bot reported invalid ID (BotName: [{}], BotID: [{}], OwnerName: [{}], OwnerID: [{}], AcctName: [{}], AcctID: [{}])",
			my_bot->GetCleanName(),
			my_bot->GetBotID(),
			c->GetCleanName(),
			c->CharacterID(),
			c->AccountName(),
			c->AccountID()
		);
		return;
	}

	if (sep->arg[1][0] == '\0' || sep->IsNumber(1)) {
		c->Message(Chat::White, "You must name your bot clone.");
		return;
	}

	std::string bot_name = sep->arg[1];

	if (!Bot::IsValidName(bot_name)) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"'{}' is an invalid name. You may only use characters 'A-Z' or 'a-z' and it must be between 4 and 15 characters. Mixed case {} allowed.",
				bot_name, RuleB(Bots, AllowCamelCaseNames) ? "is" : "is not"
			).c_str()
		);

		return;
	}

	bool available_flag = false;

	!database.botdb.QueryNameAvailability(bot_name, available_flag);

	if (!available_flag) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"The name '{}' is already being used or prohibited. Please choose a different name",
				bot_name
			).c_str()
		);

		return;
	}

	uint32 bot_count = 0;
	uint32 bot_class_count = 0;

	if (!database.botdb.QueryBotCount(c->CharacterID(), my_bot->GetClass(), bot_count, bot_class_count)) {
		c->Message(Chat::Yellow, "Failed to query bot count.");

		return;
	}

	if (!Bot::CheckCreateLimit(c, bot_count)) {
		return;
	}

	if (!Bot::CheckCreateLimit(c, bot_class_count, my_bot->GetClass())) {
		return;
	}

	uint32 clone_id = 0;

	if (!database.botdb.CreateCloneBot(my_bot->GetBotID(), bot_name, clone_id) || !clone_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to create clone bot '{}'.",
				bot_name
			).c_str()
		);
		return;
	}

	int clone_stance = Stance::Passive;

	if (!database.botdb.LoadStance(my_bot->GetBotID(), clone_stance)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to load stance from '{}'.",
				my_bot->GetCleanName()
			).c_str()
		);
	}

	if (!database.botdb.SaveStance(clone_id, clone_stance)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to save stance for clone '{}'.",
				bot_name
			).c_str()
		);
	}

	if (!database.botdb.CreateCloneBotInventory(my_bot->GetBotID(), clone_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to create clone bot inventory for clone '{}'.",
				bot_name
			).c_str()
		);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Bot Cloned | From: {} To: {}",
			my_bot->GetCleanName(),
			bot_name
		).c_str()
	);
}

void bot_command_create(Client *c, const Seperator *sep)
{
	const std::string class_substrs[17] = {
		"",
		"WAR", "CLR", "PAL", "RNG",
		"SHD", "DRU", "MNK", "BRD",
		"ROG", "SHM", "NEC", "WIZ",
		"MAG", "ENC", "BST", "BER"
	};

	const std::string race_substrs[17] = {
		"",
		"HUM", "BAR", "ERU", "ELF",
		"HIE", "DEF", "HEF", "DWF",
		"TRL", "OGR", "HFL", "GNM",
		"IKS", "VAH", "FRG", "DRK"
	};

	const uint16 race_values[17] = {
		Race::Doug,
		Race::Human, Race::Barbarian, Race::Erudite, Race::WoodElf,
		Race::HighElf, Race::DarkElf, Race::HalfElf, Race::Dwarf,
		Race::Troll, Race::Ogre, Race::Halfling, Race::Gnome,
		Race::Iksar, Race::VahShir, Race::Froglok2, Race::Drakkin
	};

	const std::string gender_substrs[2] = {
		"Male", "Female",
	};

	if (helper_command_alias_fail(c, "bot_command_create", sep->arg[0], "botcreate")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Name] [Class] [Race] [Gender]",
				sep->arg[0]
			).c_str()
		);

		std::string window_text;
		std::string message_separator;
		int object_count = 0;
		const int object_max = 4;

		window_text.append(
			fmt::format(
				"Classes{}<c \"#FFFF\">",
				DialogueWindow::Break()
			)
		);

		message_separator = " ";
		object_count = 0;
		for (int i = 0; i <= 15; ++i) {
			window_text.append(message_separator);

			if (object_count >= object_max) {
				window_text.append(DialogueWindow::Break());
				object_count = 0;
			}

			window_text.append(
				fmt::format("{} ({})",
							class_substrs[i + 1],
							(i + 1)
				)
			);

			++object_count;
			message_separator = ", ";
		}

		window_text.append(DialogueWindow::Break(2));

		window_text.append(
			fmt::format(
				"<c \"#FFFFFF\">Races{}<c \"#FFFF\">",
				DialogueWindow::Break()
			)
		);

		message_separator = " ";
		object_count = 0;
		for (int i = 0; i <= 15; ++i) {
			window_text.append(message_separator);

			if (object_count >= object_max) {
				window_text.append(DialogueWindow::Break());
				object_count = 0;
			}

			window_text.append(
				fmt::format("{} ({})",
							race_substrs[i + 1],
							race_values[i + 1]
				)
			);

			++object_count;
			message_separator = ", ";
		}

		window_text.append(DialogueWindow::Break(2));

		window_text.append(
			fmt::format(
				"<c \"#FFFFFF\">Genders{}<c \"#FFFF\">",
				DialogueWindow::Break()
			)
		);

		message_separator = " ";
		for (int i = 0; i <= 1; ++i) {
			window_text.append(message_separator);

			window_text.append(
				fmt::format("{} ({})",
							gender_substrs[i],
							i
				)
			);

			message_separator = ", ";
		}

		c->SendPopupToClient("Bot Creation Options", window_text.c_str());

		return;
	}

	const auto arguments = sep->argnum;
	if (!arguments || sep->IsNumber(1)) {
		c->Message(Chat::White, "You must name your bot!");
		return;
	}

	std::string bot_name = sep->arg[1];
	bot_name = Strings::UcFirst(bot_name);

	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Invalid class!");
		return;
	}

	auto bot_class = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));

	if (arguments < 3 || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Invalid race!");
		return;
	}

	auto bot_race = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[3]));

	if (arguments < 4) {
		c->Message(Chat::White, "Invalid gender!");
		return;
	}

	auto bot_gender = Gender::Male;

	if (sep->IsNumber(4)) {
		bot_gender = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[4]));
		if (bot_gender == Gender::Neuter) {
			bot_gender = Gender::Male;
		}
	} else {
		if (!strcasecmp(sep->arg[4], "m") || !strcasecmp(sep->arg[4], "male")) {
			bot_gender = Gender::Male;
		} else if (!strcasecmp(sep->arg[4], "f") || !strcasecmp(sep->arg[4], "female")) {
			bot_gender = Gender::Female;
		}
	}

	helper_bot_create(c, bot_name, bot_class, bot_race, bot_gender);
}

void bot_command_delete(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_delete", sep->arg[0], "botdelete"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s", sep->arg[0]);
		return;
	}

	if (!Bot::CheckCampSpawnConditions(c)) {
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	std::string delete_confirm = sep->arg[1];

	std::string deleted_check = "confirm";

	if (!(delete_confirm.find(deleted_check) != std::string::npos)) {
		c->Message(Chat::White, "You must type %s confirm to confirm the deletion of %s.", sep->arg[0], my_bot->GetCleanName());
		return;
	}

	if (!my_bot->DeleteBot()) {
		c->Message(Chat::White, "Failed to delete '%s' due to database error", my_bot->GetCleanName());
		return;
	}

	auto bid = my_bot->GetBotID();
	std::string bot_name = my_bot->GetCleanName();

	my_bot->Camp(false);

	c->Message(Chat::White, "Successfully deleted bot '%s' (id: %i)", bot_name.c_str(), bid);
}

void bot_command_follow_distance(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_follow_distance", sep->arg[0], "botfollowdistance")) {
		c->Message(Chat::White, "note: Sets or resets the follow distance of the selected bots.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Sets or resets the follow distance of the selected bots." };
		p.notes =
		{
			fmt::format("[Default]: {}", RuleI(Bots, DefaultFollowDistance)),
			fmt::format("- You must use a value between 1 and {}.", RuleI(Bots, MaxFollowDistance))
		};
		p.example_format = { fmt::format("{} [reset]/[set [value]] [actionable]", sep->arg[0]) };
		p.examples_one = {
			"To set all bots to follow at a distance of 25:",
			fmt::format("{} set 25 spawned", sep->arg[0])
		};
		p.examples_two = {
			"To check the curret following distance of all bots:",
			fmt::format("{} current spawned", sep->arg[0])
		};
		p.examples_three =
		{
			"To reset the following distance of all Wizards:",
			fmt::format(
				"{} reset byclass {}",
				sep->arg[0],
				Class::Wizard
			)
		};
		p.actionables = { "target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned" };

		std::string popup_text = c->SendBotCommandHelpWindow(p);
		popup_text = DialogueWindow::Table(popup_text);

		c->SendPopupToClient(sep->arg[0], popup_text.c_str());

		return;
	}

	const int ab_mask = ActionableBots::ABM_Type2;

	uint32 bfd = RuleI(Bots, DefaultFollowDistance);
	bool set_flag = false;
	bool current_check = false;
	int ab_arg = 2;

	std::string arg1 = sep->arg[1];

	if (!arg1.compare("set")) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::Yellow, "You must enter a value between 1 and %i.", RuleI(Bots, MaxFollowDistance));

			return;
		}

		bfd = Strings::ToInt(sep->arg[2]);

		if (bfd < 1) {
			c->Message(Chat::Yellow, "You must enter a value between 1 and %i.", RuleI(Bots, MaxFollowDistance));

			return;
		}

		if (bfd > RuleI(Bots, MaxFollowDistance)) {
			c->Message(Chat::Yellow, "You must enter a value between 1 and %i.", RuleI(Bots, MaxFollowDistance));

			return;
		}

		set_flag = true;
		++ab_arg;
	}
	else if (!arg1.compare("current")) {
		current_check = true;
	}
	else if (arg1.compare("reset")) {
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

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;
	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;

	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	int bot_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter) {
			continue;
		}

		if (current_check) {
			Mob* follow_mob = entity_list.GetMob(bot_iter->GetFollowID());

			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'I am currently following {} at a distance of {}.'",
					bot_iter->GetCleanName(),
					follow_mob ? follow_mob->GetCleanName() : "no one",
					sqrt(bot_iter->GetFollowDistance())
				).c_str()
			);
		}
		else {
			bot_iter->SetFollowDistance(bfd * bfd);
			++bot_count;
		}
	}

	if (current_check) {
		return;
	}

	if (bot_count == 1) {
		Mob* follow_mob = entity_list.GetMob(sbl.front()->GetFollowID());

		c->Message(
			Chat::Green,
			fmt::format(
				"{} says, 'Following {} at a distance of {}.'",
				sbl.front()->GetCleanName(),
				follow_mob ? follow_mob->GetCleanName() : "you",
				bfd
			).c_str()
		);
	}
	else {
		c->Message(
			Chat::Green,
			fmt::format(
				"{} of your bots are now following at a distance of {}.",
				bot_count,
				bfd
			).c_str()
		);
	}
}

void bot_command_inspect_message(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_inspect_message", sep->arg[0], "botinspectmessage"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s [set | clear] ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationmembers | healrotationtargets | mmr | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		c->Message(Chat::White, "Notes:");
		if (c->ClientVersion() >= EQ::versions::ClientVersion::SoF) {
			c->Message(Chat::White, "- Self-inspect and type your bot's inspect message");
			c->Message(Chat::White, "- Close the self-inspect window to update the server");
			c->Message(Chat::White, "- Type '%s set' to set the bot's inspect message", sep->arg[0]);
		}
		else {
			c->Message(Chat::White, "- Self-inspect and type your bot's inspect message");
			c->Message(Chat::White, "- Close the self-inspect window");
			c->Message(Chat::White, "- Self-inspect again to update the server");
			c->Message(Chat::White, "- Type '%s set' to set the bot's inspect message", sep->arg[0]);
		}
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	bool set_flag = false;
	if (!strcasecmp(sep->arg[1], "set")) {
		set_flag = true;
	}
	else if (strcasecmp(sep->arg[1], "clear")) {
		c->Message(Chat::White, "This command requires a [set | clear] argument");
		return;
	}

	std::vector<Bot*> sbl;
	auto ab_type = ActionableBots::PopulateSBL(c, sep->arg[2], sbl, ab_mask, sep->arg[3]);
	if (ab_type == ActionableBots::ABT_None)
		return;

	const auto client_message_struct = &c->GetInspectMessage();

	int bot_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		auto bot_message_struct = &bot_iter->GetInspectMessage();
		memset(bot_message_struct, 0, sizeof(InspectMessage_Struct));
		if (set_flag)
			memcpy(bot_message_struct, client_message_struct, sizeof(InspectMessage_Struct));

		if (ab_type != ActionableBots::ABT_All && !database.botdb.SaveInspectMessage(bot_iter->GetBotID(), *bot_message_struct)) {
			return;
		}

		++bot_count;
	}

	if (ab_type == ActionableBots::ABT_All) {
		InspectMessage_Struct bot_message_struct;
		memset(&bot_message_struct, 0, sizeof(InspectMessage_Struct));
		if (set_flag)
			memcpy(&bot_message_struct, client_message_struct, sizeof(InspectMessage_Struct));

		if (!database.botdb.SaveAllInspectMessages(c->CharacterID(), bot_message_struct)) {
			return;
		}

		c->Message(Chat::White, "%s all of your bot inspect messages", set_flag ? "Set" : "Cleared");
	}
	else {
		c->Message(Chat::White, "%s %i of your spawned bot inspect messages", set_flag ? "Set" : "Cleared", bot_count);
	}
}

void bot_command_list_bots(Client *c, const Seperator *sep)
{
	enum {
		FilterClass,
		FilterRace,
		FilterName,
		FilterCount,
		MaskClass = (1 << FilterClass),
		MaskRace = (1 << FilterRace),
		MaskName = (1 << FilterName)
	};

	if (helper_command_alias_fail(c, "bot_command_list", sep->arg[0], "botlist")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} (account) ([class] [value]) ([race] [value]) ([name] [partial-full])",
				sep->arg[0]
			).c_str()
		);
		c->Message(Chat::White, "Note: Filter criteria is orderless and optional.");
		return;
	}

    int NO_BOT_LIMIT = -1;
	bool Account = false;
	int seps = 1;
	uint32 filter_value[FilterCount];
	int name_criteria_arg = 0;
	memset(&filter_value, 0, sizeof(uint32) * FilterCount);

	int filter_mask = 0;
	if (!strcasecmp(sep->arg[1], "account")) {
		Account = true;
		seps = 2;
	}

	for (int i = seps; i < (FilterCount * 2); i += 2) {
		if (sep->arg[i][0] == '\0') {
			break;
		}

		if (!strcasecmp(sep->arg[i], "class")) {
			filter_mask |= MaskClass;
			filter_value[FilterClass] = Strings::ToInt(sep->arg[i + 1]);
			continue;
		}

		if (!strcasecmp(sep->arg[i], "race")) {
			filter_mask |= MaskRace;
			filter_value[FilterRace] = Strings::ToInt(sep->arg[i + 1]);
			continue;
		}

		if (!strcasecmp(sep->arg[i], "name")) {
			filter_mask |= MaskName;
			name_criteria_arg = (i + 1);
			continue;
		}

		c->Message(Chat::White, "A numeric value or name is required to use the filter property of this command (f: '%s', v: '%s')", sep->arg[i], sep->arg[i + 1]);
		return;
	}

	std::list<BotsAvailableList> bots_list;
	if (!database.botdb.LoadBotsList(c->CharacterID(), bots_list, Account)) {
		return;
	}

	if (bots_list.empty()) {
		c->Message(Chat::White, "You have no bots.");
		return;
	}

	auto bot_count = 0;
	auto bots_owned = 0;
	auto bot_number = 1;
	for (auto bots_iter : bots_list) {
		if (filter_mask) {
			if ((filter_mask & MaskClass) && filter_value[FilterClass] != bots_iter.class_) {
				continue;
			}

			if ((filter_mask & MaskRace) && filter_value[FilterRace] != bots_iter.race) {
				continue;
			}

			if (filter_mask & MaskName) {
				std::string name_criteria = sep->arg[name_criteria_arg];
				std::transform(name_criteria.begin(), name_criteria.end(), name_criteria.begin(), ::tolower);
				std::string name_check = bots_iter.bot_name;
				std::transform(name_check.begin(), name_check.end(), name_check.begin(), ::tolower);
				if (name_check.find(name_criteria) == std::string::npos) {
					continue;
				}
			}
		}

		auto* bot = entity_list.GetBotByBotName(bots_iter.bot_name);

		c->Message(
			Chat::White,
			fmt::format(
				"Bot {} | {} is a Level {} {} {} {} owned by {}.",
				bot_number,
				(
					(c->CharacterID() == bots_iter.owner_id && !bot) ?
						Saylink::Silent(
							fmt::format("^spawn {}", bots_iter.bot_name),
							bots_iter.bot_name
						) :
						bots_iter.bot_name
				),
				bots_iter.level,
				GetGenderName(bots_iter.gender),
				GetRaceIDName(bots_iter.race),
				GetClassIDName(bots_iter.class_),
				bots_iter.owner_name
			).c_str()
		);

		if (c->CharacterID() == bots_iter.owner_id) {
			bots_owned++;
		}

		bot_count++;
		bot_number++;
	}

	if (!bot_count) {
		c->Message(Chat::White, "You have no bots meeting this criteria.");
		return;
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} Of {} bot{} shown, {} {} owned by you.",
				bot_count,
				bots_list.size(),
				bot_count != 1 ? "s" : "",
				bots_owned,
				bots_owned != 1 ? "are" : "is"
			).c_str()
		);

		c->Message(Chat::White, "Note: You can spawn any owned bots by clicking their name if they are not already spawned.");

		c->Message(Chat::White, "Your bot creation limits are as follows:");

		const auto overall_bot_creation_limit = c->GetBotCreationLimit();

		c->Message(
			Chat::White,
			fmt::format(
				"Overall | {} Bot{}",
				overall_bot_creation_limit,
				overall_bot_creation_limit != 1 ? "s" : ""
			).c_str()
		);

		for (uint8 class_id = Class::Warrior; class_id <= Class::Berserker; class_id++) {
			auto class_creation_limit = c->GetBotCreationLimit(class_id);

			if (class_creation_limit != NO_BOT_LIMIT && class_creation_limit != overall_bot_creation_limit) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} | {} Bot{}",
						GetClassIDName(class_id),
						class_creation_limit,
						class_creation_limit != 1 ? "s" : ""
					).c_str()
				);
			}
		}
	}
}

void bot_command_report(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_report", sep->arg[0], "botreport"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationmembers | healrotationtargets | mmr | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}

	const int ab_mask = ActionableBots::ABM_Type1;

	std::string arg1 = sep->arg[1];
	int ab_arg = 1;

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;

	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		std::string report_msg = StringFormat("%s %s reports", GetClassIDName(bot_iter->GetClass()), bot_iter->GetCleanName());
		report_msg.append(StringFormat(": %3.1f%% health", bot_iter->GetHPRatio()));
		if (!IsNonSpellFighterClass(bot_iter->GetClass()))
			report_msg.append(StringFormat(": %3.1f%% mana", bot_iter->GetManaRatio()));

		c->Message(Chat::White, "%s", report_msg.c_str());
	}
}

void bot_command_spawn(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_spawn", sep->arg[0], "botspawn")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [bot_name] [optional: silent]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	if (!Bot::CheckHighEnoughLevelForBots(c)) {
		return;
	}

	if (!Bot::CheckCampSpawnConditions(c)) {
		return;
	}

	if (!Bot::CheckSpawnLimit(c)) {
		return;
	}

	if (sep->arg[1][0] == '\0' || sep->IsNumber(1)) {
		c->Message(Chat::White, "You must specify a name to use this command.");
		return;
	}

	std::string bot_name = sep->arg[1];

	uint32 bot_id = 0;
	uint8 bot_class = Class::None;

	if (!database.botdb.LoadBotID(bot_name, bot_id, bot_class)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to load bot ID for '{}'.",
				bot_name
			).c_str()
		);
		return;
	}

	if (!bot_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"You don't own a bot named '{}'.",
				bot_name
			).c_str()
		);
		return;
	}

	if (!Bot::CheckHighEnoughLevelForBots(c, bot_class)) {
		return;
	}

	if (!Bot::CheckSpawnLimit(c, bot_class)) {
		return;
	}

	if (entity_list.GetMobByBotID(bot_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"'{}' is already spawned.",
				bot_name
			).c_str()
		);

		return;
	}

	auto my_bot = Bot::LoadBot(bot_id);

	if (!my_bot) {
		c->Message(
			Chat::White,
			fmt::format(
				"Invalid bot '{}' (ID {})",
				bot_name,
				bot_id
			).c_str()
		);

		return;
	}

	if (!my_bot->Spawn(c)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to spawn '{}' (ID {})",
				bot_name,
				bot_id
			).c_str()
		);

		safe_delete(my_bot);

		return;
	}

	static std::string bot_spawn_message[17] = {
		"I am ready to fight!", // DEFAULT
		"A solid weapon is my ally!", // Class::Warrior
		"The pious shall never die!", // Class::Cleric
		"I am the symbol of Light!", // Class::Paladin
		"There are enemies near!", // Class::Ranger
		"Out of the shadows, I step!", // Class::ShadowKnight
		"Nature's fury shall be wrought!", // Class::Druid
		"Your punishment will be my fist!", // Class::Monk
		"Music is the overture of battle! ", // BARD
		"Daggers into the backs of my enemies!", // Class::Rogue
		"More bones to grind!", // Class::Shaman
		"Death is only the beginning!", // Class::Necromancer
		"I am the harbinger of demise!", // Class::Wizard
		"The elements are at my command!", // Class::Magician
		"No being can resist my charm!", // Class::Enchanter
		"Battles are won by hand and paw!", // Class::Beastlord
		"My bloodthirst shall not be quenched!" // Class::Berserker
	};

	uint8 message_index = 0;

	if (c->GetBotOption(Client::booSpawnMessageClassSpecific)) {
		message_index = VALIDATECLASSID(my_bot->GetClass());
	}

	std::string silent_confirm = sep->arg[2];
	bool silent_tell = false;

	if (!silent_confirm.compare("silent")) {
		silent_tell = true;
	}

	if (!silent_tell && c->GetBotOption(Client::booSpawnMessageSay)) {
		my_bot->RaidGroupSay(bot_spawn_message[message_index].c_str());
	}
	else if (!silent_tell && c->GetBotOption(Client::booSpawnMessageTell)) {
		my_bot->OwnerMessage(bot_spawn_message[message_index]);
	}
}

void bot_command_stance(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_stance", sep->arg[0], "botstance")) {
		c->Message(Chat::White, "note: Change a bot's stance to control the way it behaves.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Change a bot's stance to control the way it behaves." };
		p.notes =
		{
			"- Changing a stance will reset all settings to match that stance type.",
			"- Any changes made will only save to that stance for future use.",
			fmt::format(
				"- {} (#{}) will tell Non-Warrior classes to taunt automatically.",
				Stance::GetName(Stance::Aggressive),
				Stance::Aggressive
			),
			"<br>",
			"Available stances:",
			fmt::format(
				"{} (#{}), {} (#{}), {} (#{}), {} (#{}), {} (#{}), {} (#{}), {} (#{})",
				Stance::GetName(Stance::Passive),
				Stance::Passive,
				Stance::GetName(Stance::Balanced),
				Stance::Balanced,
				Stance::GetName(Stance::Efficient),
				Stance::Efficient,
				Stance::GetName(Stance::Aggressive),
				Stance::Aggressive,
				Stance::GetName(Stance::Assist),
				Stance::Assist,
				Stance::GetName(Stance::Burn),
				Stance::Burn,
				Stance::GetName(Stance::AEBurn),
				Stance::AEBurn
			),
			"<br>",
			fmt::format(
				"- {} (#{}) - Idle. Does not cast or engage in combat.",
				Stance::GetName(Stance::Passive),
				Stance::Passive
			),
			fmt::format(
				"- {} (#{}) [Default] - Overall balance and casts most spell types by default.",
				Stance::GetName(Stance::Balanced),
				Stance::Balanced
			),
			fmt::format(
				"- {} (#{}) - More mana and aggro efficient (SKs will still cast hate line). Longer delays between detrimental spells, thresholds adjusted to cast less often.",
				Stance::GetName(Stance::Efficient),
				Stance::Efficient
			),
			fmt::format(
				"- {} (#{}) - Much more aggressive in their cast times and thresholds. More DPS, debuffs and slow but a higher risk of snagging aggro.",
				Stance::GetName(Stance::Aggressive),
				Stance::Aggressive
			),
			fmt::format(
				"- {} (#{}) - Support role. Most offensive spell types are disabled. Focused on heals, cures, CC, debuffs and slows.",
				Stance::GetName(Stance::Assist),
				Stance::Assist
			),
			fmt::format(
				"- {} (#{}) - Murder. Doesn't care about aggro, just wants to kill. DPS Machine.",
				Stance::GetName(Stance::Burn),
				Stance::Burn
			),
			fmt::format(
				"- {} (#{}) - Murder EVERYTHING. Doesn't care about aggro, casts AEs. Everything must die ASAP.",
				Stance::GetName(Stance::AEBurn),
				Stance::AEBurn
			)
		};
		p.example_format =
		{ fmt::format( "{} [current | value]", sep->arg[0]) };
		p.examples_one =
		{
			"To set all bots to BurnAE:",
			fmt::format("{} {} spawned {}",
				sep->arg[0],
				Stance::Aggressive,
				Class::ShadowKnight
			)
		};
		p.examples_two =
		{
			"To set all Shadowknights to Aggressive:",
			fmt::format("{} {} byclass {}",
				sep->arg[0],
				Stance::Aggressive,
				Class::ShadowKnight
			)
		};
		p.examples_three = {
			"To check the current stances of all bots:",
			fmt::format("{} current spawned", sep->arg[0])
		};

		p.actionables = { "target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned" };

		std::string popup_text = c->SendBotCommandHelpWindow(p);
		popup_text = DialogueWindow::Table(popup_text);

		c->SendPopupToClient(sep->arg[0], popup_text.c_str());

		return;
	}

	const int ab_mask = ActionableBots::ABM_Type1;

	bool current_check = false;
	int ab_arg = 1;
	uint32 value = 0;

	std::string arg1 = sep->arg[1];

	if (sep->IsNumber(1)) {
		++ab_arg;
		value = atoi(sep->arg[1]);
		if (!Bot::IsValidBotStance(value)) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"You must choose a valid stance ID, use {} for information regarding this command.",
					Saylink::Silent(
						fmt::format("{} help", sep->arg[0])
					)
				).c_str()
			);

			return;
		}
	}
	else if (!arg1.compare("current")) {
		++ab_arg;
		current_check = true;
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

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;

	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	Bot* first_found = nullptr;
	int success_count = 0;
	for (auto bot_iter : sbl) {
		if (!first_found) {
			first_found = bot_iter;
		}

		if (current_check) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'My current stance is {} ({}).'",
					bot_iter->GetCleanName(),
					Stance::GetName(bot_iter->GetBotStance()),
					bot_iter->GetBotStance()
				).c_str()
			);

			continue;
		}

		bot_iter->Save();
		bot_iter->SetBotStance(value);
		bot_iter->LoadDefaultBotSettings();
		database.botdb.LoadBotSettings(bot_iter);

		if (
			(bot_iter->GetClass() == Class::Warrior || bot_iter->GetClass() == Class::Paladin || bot_iter->GetClass() == Class::ShadowKnight) &&
			(bot_iter->GetBotStance() == Stance::Aggressive)
		) {
			bot_iter->SetTaunting(true);

			if (bot_iter->HasPet() && bot_iter->GetPet()->GetSkill(EQ::skills::SkillTaunt)) {
				bot_iter->GetPet()->CastToNPC()->SetTaunting(true);
			}
		}
		else {
			bot_iter->SetTaunting(false);

			if (bot_iter->HasPet() && bot_iter->GetPet()->GetSkill(EQ::skills::SkillTaunt)) {
				bot_iter->GetPet()->CastToNPC()->SetTaunting(false);
			}
		}

		bot_iter->Save();
		++success_count;
	}

	if (current_check) {
		return;
	}

	if (success_count == 1 && first_found) {
		c->Message(
			Chat::Green,
			fmt::format(
				"{} says, 'I am now set to the stance [{}].'",
				first_found->GetCleanName(),
				Stance::GetName(value)
			).c_str()
		);
	}
	else {
		c->Message(
			Chat::Green,
			fmt::format(
				"{} of your bots are now set to the stance [{}].",
				success_count,
				Stance::GetName(value)
			).c_str()
		);
	}
}

void bot_command_stop_melee_level(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_stop_melee_level", sep->arg[0], "botstopmeleelevel")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s [current | reset | sync | value: 0-255] ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | mmr | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		c->Message(Chat::White, "note: Use [reset] to set stop melee level to server rule");
		c->Message(Chat::White, "note: Use [sync] to set stop melee level to current bot level");
		return;
	}

	const int ab_mask = ActionableBots::ABM_Type1;

	std::string arg1 = sep->arg[1];
	int ab_arg = 1;
	uint8 sml = RuleI(Bots, CasterStopMeleeLevel);
	bool sync_sml = false;
	bool reset_sml = false;
	bool current_check = false;

	if (sep->IsNumber(1)) {
		ab_arg = 2;
		sml = Strings::ToInt(sep->arg[1]);
		if (sml <= 0 || sml > 255) {
			c->Message(Chat::White, "You must provide a value between 0-255.");
			return;
		}
	}
	else if (!strcasecmp(sep->arg[1], "sync")) {
		ab_arg = 2;
		sync_sml = true;
	}
	else if (!arg1.compare("current")) {
		ab_arg = 2;
		current_check = true;
	}
	else if (!strcasecmp(sep->arg[1], "reset")) {
		ab_arg = 2;
		reset_sml = true;
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

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;

	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;

	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	Bot* first_found = nullptr;
	int success_count = 0;

	for (auto my_bot : sbl) {
		if (my_bot->BotPassiveCheck()) {
			continue;
		}

		if (!first_found) {
			first_found = my_bot;
		}

		if (sync_sml) {
			sml = my_bot->GetLevel();
		}

		if (reset_sml) {
			sml = my_bot->GetDefaultBotBaseSetting(BotBaseSettings::StopMeleeLevel);
		}

		if (current_check) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} says, 'My current stop melee level is {}.'",
					my_bot->GetCleanName(),
					my_bot->GetStopMeleeLevel()
				).c_str()
			);
			continue;
		}
		else {
			my_bot->SetStopMeleeLevel(sml);
			++success_count;
		}
	}

	if (!current_check) {
		if (success_count == 1 && first_found) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} says, 'My stop melee level was {} to {}.'",
					first_found->GetCleanName(),
					reset_sml ? "reset" : "set",
					sml
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::White,
				fmt::format(
					"{} of your bots {} their stop melee{}'", // level to {}.
					success_count,
					reset_sml ? "reset" : "set",
					fmt::format("{}", reset_sml ? "." : fmt::format(" level to {}.", sml).c_str()).c_str(),
					sml
				).c_str()
			);
		}
	}
}

void bot_command_summon(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_summon", sep->arg[0], "botsummon")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationtargets | mmr | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}

	const int ab_mask = ActionableBots::ABM_Type1;

	std::string arg1 = sep->arg[1];
	int ab_arg = 1;

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;

	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	for (auto bot_iter : sbl) {
		if (!bot_iter) {
			continue;
		}

		bot_iter->WipeHateList();
		bot_iter->SetTarget(nullptr);
		bot_iter->Teleport(c->GetPosition());
		bot_iter->DoAnim(0);

		if (!bot_iter->HasPet()) {
			continue;
		}

		if (bot_iter->HasControllablePet(BotAnimEmpathy::BackOff)) {
			bot_iter->GetPet()->WipeHateList();
			bot_iter->GetPet()->SetTarget(nullptr);
		}

		bot_iter->GetPet()->Teleport(c->GetPosition());
	}

	if (sbl.size() == 1) {
		c->Message(
			Chat::White,
			fmt::format(
				"Summoned {} to you.",
				sbl.front() ? sbl.front()->GetCleanName() : "no one"
			).c_str()
		);
	}
	else {
		c->Message(
			Chat::White,
			fmt::format(
				"Summoned {} bots to you.",
				sbl.size()
			).c_str()
		);
	}
}

void bot_command_toggle_ranged(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_toggle_ranged", sep->arg[0], "bottoggleranged")) {
		c->Message(Chat::White, "note: Toggles a ranged bot between melee and ranged weapon use.");

		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Toggles a ranged bot between melee and ranged weapon use." };
		p.example_format = { fmt::format("{} [value] [actionable]", sep->arg[0]) };
		p.examples_one =
		{
			"To set BotA to use their ranged:",
			fmt::format(
				"{} 1 byname BotA",
				sep->arg[0]
			)
		};
		p.examples_two =
		{
			"To set all ranger bots to ranged:",
			fmt::format(
				"{} 1 byclass {}",
				sep->arg[0],
				Class::Ranger
			)
		};
		p.examples_two =
		{
			"To check the ranged status of all bots:",
			fmt::format(
				"{} current spawned",
				sep->arg[0],
				Class::Ranger
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

	int ab_arg = 1;
	bool current_check = false;
	uint32 type_value = 0;

	if (sep->IsNumber(1)) {
		type_value = atoi(sep->arg[1]);
		++ab_arg;
		if (type_value < 0 || type_value > 1) {
			c->Message(Chat::Yellow, "You must enter either 0 for disabled or 1 for enabled.");

			return;
		}
	}
	else if (!arg1.compare("current")) {
		++ab_arg;
		current_check = true;
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

	const int ab_mask = ActionableBots::ABM_Type1;
	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;

	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	Bot* first_found = nullptr;
	int success_count = 0;
	for (auto my_bot : sbl) {
		if (my_bot->BotPassiveCheck()) {
			continue;
		}

		if (current_check) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'I {} currently ranged.'",
					my_bot->GetCleanName(),
					my_bot->IsBotRanged() ? "am" : "am no longer"
				).c_str()
			);
		}
		else {
			if (my_bot->GetBotRangedValue() < RuleI(Combat, MinRangedAttackDist)) {
				c->Message(Chat::Yellow, "%s does not have proper weapons or ammo to be at range.", my_bot->GetCleanName());
				continue;
			}

			if (!first_found) {
				first_found = my_bot;
			}

			my_bot->SetBotRangedSetting(type_value);
			my_bot->ChangeBotRangedWeapons(my_bot->IsBotRanged());
			++success_count;
		}
	}

	if (!current_check) {
		if (success_count == 1 && first_found) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'I {} ranged.'",
					first_found->GetCleanName(),
					first_found->IsBotRanged() ? "am now" : "am no longer"
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} of your bots {} ranged.",
					success_count,
					type_value ? "are now" : "are no longer"
				).c_str()
			);
		}
	}
}

void bot_command_toggle_helm(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_toggle_helm", sep->arg[0], "bottogglehelm")) {
		c->Message(Chat::White, "note: Toggles whether or not bots will show their helm.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Toggles whether or not bots will show their helm." };
		p.example_format = { fmt::format("{} [value] [actionable]", sep->arg[0]) };
		p.examples_one =
		{
			"To set BotA to show their helm:",
			fmt::format(
				"{} 1 byname BotA",
				sep->arg[0]
			)
		};
		p.examples_two =
		{
			"To set all bots to show their helm:",
			fmt::format(
				"{} 1 spawned",
				sep->arg[0]
			)
		};
		p.examples_three =
		{
			"To check the toggle helm status of all bots:",
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

	int ab_arg = 1;
	bool current_check = false;
	uint32 type_value = 0;

	if (sep->IsNumber(1)) {
		type_value = atoi(sep->arg[1]);
		++ab_arg;
		if (type_value < 0 || type_value > 1) {
			c->Message(Chat::Yellow, "You must enter either 0 for disabled or 1 for enabled.");

			return;
		}
	}
	else if (!arg1.compare("current")) {
		++ab_arg;
		current_check = true;
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

	const int ab_mask = ActionableBots::ABM_Type1;
	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;

	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	Bot* first_found = nullptr;
	int success_count = 0;
	for (auto my_bot : sbl) {
		if (my_bot->BotPassiveCheck()) {
			continue;
		}

		if (!first_found) {
			first_found = my_bot;
		}

		if (current_check) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'I {} show my helm.'",
					my_bot->GetCleanName(),
					my_bot->GetShowHelm() ? "will" : "will not"
				).c_str()
			);
		}
		else {
			my_bot->SetShowHelm(type_value);

			auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
			SpawnAppearance_Struct* saptr = (SpawnAppearance_Struct*) outapp->pBuffer;
			saptr->spawn_id = my_bot->GetID();
			saptr->type = AppearanceType::ShowHelm;
			saptr->parameter = my_bot->GetShowHelm();

			entity_list.QueueClients(my_bot, outapp, true);
			safe_delete(outapp);

			++success_count;
		}
	}
	if (!current_check) {
		if (success_count == 1 && first_found) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'I {} show my helm.'",
					first_found->GetCleanName(),
					first_found->GetShowHelm() ? "will now" : "will no longer"
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} of your bots {} show their helm.",
					success_count,
					type_value ? "will now" : "will no longer"
				).c_str()
			);
		}
	}

	// Notes:
	/*
	[CLIENT OPCODE TEST]
	[10-16-2015 :: 14:57:56] [Packet :: Client -> Server (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 01 00 00 00 - showhelm = true (client)
	[10-16-2015 :: 14:57:56] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 01 00 00 00 - showhelm = true (client)

	[10-16-2015 :: 14:58:02] [Packet :: Client -> Server (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 00 00 00 00 - showhelm = false (client)
	[10-16-2015 :: 14:58:02] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 00 00 00 00 - showhelm = false (client)

	[BOT OPCODE TEST]
	[10-16-2015 :: 22:15:34] [Packet :: Client -> Server (Dump)] [OP_ChannelMessage - 0x0045] [Size: 167]
	0: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	16: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	32: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	48: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	64: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	80: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	96: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	112: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	128: 00 00 00 00 08 00 00 00 - CD CD CD CD CD CD CD CD  | ................
	144: 64 00 00 00 23 62 6F 74 - 20 73 68 6F 77 68 65 6C  | d...#bot showhel
	160: 6D 20 6F 6E 00                                     | m on.

	[10-16-2015 :: 22:15:34] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A2 02 2B 00 01 00 00 00 - showhelm = true

	[10-16-2015 :: 22:15:40] [Packet :: Client -> Server (Dump)] [OP_ChannelMessage - 0x0045] [Size: 168]
	0: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	16: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	32: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	48: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	64: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	80: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	96: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	112: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	128: 00 00 00 00 08 00 00 00 - CD CD CD CD CD CD CD CD  | ................
	144: 64 00 00 00 23 62 6F 74 - 20 73 68 6F 77 68 65 6C  | d...#bot showhel
	160: 6D 20 6F 66 66 00                                  | m off.

	[10-16-2015 :: 22:15:40] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A2 02 2B 00 00 00 00 00 - showhelm = false

	*** Bot did not update using the OP_SpawnAppearance packet with AppearanceType::ShowHelm appearance type ***
	*/
}

void bot_command_update(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_update", sep->arg[0], "botupdate"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s", sep->arg[0]);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);
	if (sbl.empty()) {
		c->Message(Chat::White, "You currently have no spawned bots");
		return;
	}

	int bot_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter || bot_iter->IsEngaged() || bot_iter->GetLevel() == c->GetLevel())
			continue;

		bot_iter->CalcBotStats(c->GetBotOption(Client::booStatsUpdate));
		bot_iter->SendAppearancePacket(AppearanceType::WhoLevel, bot_iter->GetLevel(), true, true);
		++bot_count;
	}

	c->Message(Chat::White, "Updated %i of your %i spawned bots", bot_count, sbl.size());
}
