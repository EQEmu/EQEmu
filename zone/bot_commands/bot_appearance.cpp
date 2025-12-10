#include "../bot_command.h"

void bot_command_appearance(Client *c, const Seperator *sep)
{
	std::vector<const char*> subcommand_list = {
		"botbeardcolor",
		"botbeardstyle",
		"botdetails",
		"botdyearmor",
		"boteyes",
		"botface",
		"bothaircolor",
		"bothairstyle",
		"botheritage",
		"bottattoo",
		"botwoad"
	};

	if (helper_command_alias_fail(c, "bot_command_appearance", sep->arg[0], "botappearance"))
		return;

	helper_send_available_subcommands(c, "bot appearance", subcommand_list);
}

void bot_command_beard_color(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_beard_color", sep->arg[0], "botbeardcolor"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n] (Dwarves or male bots only)", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = AFT_None;
	if (my_bot->GetGender() != Gender::Male && my_bot->GetRace() != Race::Dwarf)
		fail_type = AFT_GenderRace;
	else if (!RaceAppearance::IsValidBeardColor(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = AFT_Value;
	else
		my_bot->SetBeardColor(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "beard color"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_beard_style(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_beard_style", sep->arg[0], "botbeardstyle"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n] (Dwarves or male bots only)", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = AFT_None;
	if (my_bot->GetGender() != Gender::Male && my_bot->GetRace() != Race::Dwarf)
		fail_type = AFT_GenderRace;
	else if (!RaceAppearance::IsValidBeard(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = AFT_Value;
	else
		my_bot->SetBeard(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "beard style"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_details(Client *c, const Seperator *sep)
{
	// TODO: Trouble-shoot model update issue

	if (helper_command_alias_fail(c, "bot_command_details", sep->arg[0], "botdetails"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n] (Drakkin bots only)", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint32 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = AFT_None;
	if (my_bot->GetRace() != Race::Drakkin)
		fail_type = AFT_Race;
	else if (!RaceAppearance::IsValidDetail(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = AFT_Value;
	else
		my_bot->SetDrakkinDetails(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "details"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_dye_armor(Client *c, const Seperator *sep)
{
	// TODO: Trouble-shoot model update issue

	const std::string material_slot_message = fmt::format(
		"Material Slots: * (All), {} (Head), {} (Chest), {} (Arms), {} (Wrists), {} (Hands), {} (Legs), {} (Feet)",
		EQ::textures::armorHead,
		EQ::textures::armorChest,
		EQ::textures::armorArms,
		EQ::textures::armorWrist,
		EQ::textures::armorHands,
		EQ::textures::armorLegs,
		EQ::textures::armorFeet
	);

	if (helper_command_alias_fail(c, "bot_command_dye_armor", sep->arg[0], "botdyearmor")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1]) || !sep->arg[1] || (sep->arg[1] && !Strings::IsNumber(sep->arg[1]))) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Material Slot] [Red: 0-255] [Green: 0-255] [Blue: 0-255] ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationmembers | healrotationtargets | mmr | byclass | byrace | spawned] ([actionable_name]))",
				sep->arg[0]
			).c_str()
		);
		c->Message(Chat::White, material_slot_message.c_str());
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	uint8 material_slot = EQ::textures::materialInvalid;
	int16 slot_id = INVALID_INDEX;

	bool dye_all = (sep->arg[1][0] == '*');
	if (!dye_all) {
		material_slot = Strings::ToInt(sep->arg[1]);
		slot_id = EQ::InventoryProfile::CalcSlotFromMaterial(material_slot);

		if (!sep->IsNumber(1) || slot_id == INVALID_INDEX || material_slot > EQ::textures::LastTintableTexture) {
			c->Message(Chat::White, "Valid material slots for this command are:");
			c->Message(Chat::White, material_slot_message.c_str());
			return;
		}
	}

	if (!sep->IsNumber(2)) {
		c->Message(Chat::White, "Valid Red values for this command are 0 to 255.");
		return;
	}

	uint32 red_value = Strings::ToUnsignedInt(sep->arg[2]);
	if (red_value > 255) {
		red_value = 255;
	}

	if (!sep->IsNumber(3)) {
		c->Message(Chat::White, "Valid Green values for this command are 0 to 255.");
		return;
	}

	uint32 green_value = Strings::ToUnsignedInt(sep->arg[3]);
	if (green_value > 255) {
		green_value = 255;
	}

	if (!sep->IsNumber(4)) {
		c->Message(Chat::White, "Valid Blue values for this command are 0 to 255.");
		return;
	}

	uint32 blue_value = Strings::ToUnsignedInt(sep->arg[4]);
	if (blue_value > 255) {
		blue_value = 255;
	}

	uint32 rgb_value = ((uint32)red_value << 16) | ((uint32)green_value << 8) | ((uint32)blue_value);

	std::vector<Bot*> sbl;
	auto ab_type = ActionableBots::PopulateSBL(c, sep->arg[5], sbl, ab_mask);
	if (ab_type == ActionableBots::ABT_None) {
		return;
	}

	for (auto bot_iter : sbl) {
		if (!bot_iter) {
			continue;
		}

		if (!bot_iter->DyeArmor(slot_id, rgb_value, dye_all, (ab_type != ActionableBots::ABT_All))) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to change armor color for {} due to unknown cause.",
					bot_iter->GetCleanName()
				).c_str()
			);
			return;
		}
	}

	if (ab_type == ActionableBots::ABT_All) {
		if (dye_all) {
			database.botdb.SaveAllArmorColors(c->CharacterID(), rgb_value);
		} else {
			database.botdb.SaveAllArmorColorBySlot(c->CharacterID(), slot_id, rgb_value);
		}
	}
}

void bot_command_eyes(Client *c, const Seperator *sep)
{
	// TODO: Trouble-shoot model update issue

	// not sure if left/right bias is allowed in pc-type entities (something is keeping them from being different)
	if (helper_command_alias_fail(c, "bot_command_eyes", sep->arg[0], "boteyes"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		//c->Message(Chat::White, "usage: <target_bot> %s [value:0-n] ([option: left | right])", sep->arg[0]);
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n]", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = Strings::ToInt(sep->arg[1]);

	//uint8 eye_bias = 0;
	//std::string arg2 = sep->arg[2];
	//if (!arg2.compare("left"))
	//	eye_bias = 1;
	//else if (!arg2.compare("right"))
	//	eye_bias = 2;

	auto fail_type = AFT_None;
	if (!RaceAppearance::IsValidEyeColor(my_bot->GetRace(), my_bot->GetGender(), uvalue)) {
		fail_type = AFT_Value;
	}
	else {
		//if (eye_bias == 1) {
		//	my_bot->SetEyeColor1(uvalue);
		//}
		//else if (eye_bias == 2) {
		//	my_bot->SetEyeColor2(uvalue);
		//}
		//else {
		my_bot->SetEyeColor1(uvalue);
		my_bot->SetEyeColor2(uvalue);
		//}
	}

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "eyes"))
		return;

	c->Message(Chat::White, "This feature will update the next time your bot is spawned");
	//helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_face(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_face", sep->arg[0], "botface"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n]", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = AFT_None;
	if (!RaceAppearance::IsValidFace(my_bot->GetRace(), my_bot->GetGender(), uvalue)) {
		fail_type = AFT_Value;
	}
	else {
		uint8 old_woad = 0;
		if (my_bot->GetRace() == Race::Barbarian)
			old_woad = ((my_bot->GetLuclinFace() / 10) * 10);
		my_bot->SetLuclinFace((old_woad + uvalue));
	}

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "face"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_hair_color(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_hair_color", sep->arg[0], "bothaircolor"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n]", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = AFT_None;
	if (!RaceAppearance::IsValidHairColor(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = AFT_Value;
	else
		my_bot->SetHairColor(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "hair color"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_hairstyle(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_hairstyle", sep->arg[0], "bothairstyle"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n]", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = AFT_None;
	if (!RaceAppearance::IsValidHair(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = AFT_Value;
	else
		my_bot->SetHairStyle(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "hair style"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_heritage(Client *c, const Seperator *sep)
{
	// TODO: Trouble-shoot model update issue

	if (helper_command_alias_fail(c, "bot_command_heritage", sep->arg[0], "botheritage"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n] (Drakkin bots only)", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint32 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = AFT_None;
	if (my_bot->GetRace() != Race::Drakkin)
		fail_type = AFT_Race;
	else if (!RaceAppearance::IsValidHeritage(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = AFT_Value;
	else
		my_bot->SetDrakkinHeritage(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "heritage"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_tattoo(Client *c, const Seperator *sep)
{
	// TODO: Trouble-shoot model update issue

	if (helper_command_alias_fail(c, "bot_command_tattoo", sep->arg[0], "bottattoo"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n] (Drakkin bots only)", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint32 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = AFT_None;
	if (my_bot->GetRace() != Race::Drakkin)
		fail_type = AFT_Race;
	else if (!RaceAppearance::IsValidTattoo(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = AFT_Value;
	else
		my_bot->SetDrakkinTattoo(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "tattoo"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_woad(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_woad", sep->arg[0], "botwoad"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n] (Barbarian bots only)", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = AFT_None;
	if (my_bot->GetRace() != Race::Barbarian) {
		fail_type = AFT_Race;
	}
	else if (!RaceAppearance::IsValidWoad(my_bot->GetRace(), my_bot->GetGender(), uvalue)) {
		fail_type = AFT_Value;
	}
	else {
		uint8 old_face = (my_bot->GetLuclinFace() % 10);
		my_bot->SetLuclinFace(((uvalue * 10) + old_face));
	}

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "woad"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}
