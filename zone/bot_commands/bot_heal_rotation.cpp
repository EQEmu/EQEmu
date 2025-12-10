#include "../bot_command.h"

void bot_command_heal_rotation(Client *c, const Seperator *sep)
{
	std::vector<const char*> subcommand_list = {
		"healrotationadaptivetargeting",
		"healrotationaddmember",
		"healrotationaddtarget",
		"healrotationadjustcritical",
		"healrotationadjustsafe",
		"healrotationcastoverride",
		"healrotationchangeinterval",
		"healrotationclearhot",
		"healrotationcleartargets",
		"healrotationcreate",
		"healrotationdelete",
		"healrotationfastheals",
		"healrotationlist",
		"healrotationremovemember",
		"healrotationremovetarget",
		"healrotationresetlimits",
		"healrotationsave",
		"healrotationsethot",
		"healrotationstart",
		"healrotationstop"
	};

	if (helper_command_alias_fail(c, "bot_command_heal_rotation", sep->arg[0], "healrotation"))
		return;

#if (EQDEBUG >= 12)
	while (c->Admin() >= AccountStatus::GMImpossible) {
		if (strcasecmp(sep->arg[1], "shone")) { break; }
		Bot* my_bot = ActionableBots::AsTarget_ByBot(c);
		if (!my_bot || !(my_bot->IsHealRotationMember())) { break; }
		auto tlist = (*my_bot->MemberOfHealRotation())->TargetList();
		if (tlist->empty()) { break; }
		for (auto tlist_iter : *tlist) {
			if (tlist_iter)
				tlist_iter->SetHP((tlist_iter->GetMaxHP() / 100 + 1));
		}
		return;
	}
#endif

	helper_send_available_subcommands(c, "bot heal rotation", subcommand_list);
}

void bot_command_heal_rotation_adaptive_targeting(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(
		c,
		"bot_command_heal_rotation_adaptive_targeting",
		sep->arg[0],
		"healrotationadaptivetargeting"
	)) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s ([member_name]) ([option: on | off])", sep->arg[0]);
		return;
	}

	std::string adaptive_targeting_arg;

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (!sbl.empty()) {
		adaptive_targeting_arg = sep->arg[2];
	} else {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
		adaptive_targeting_arg = sep->arg[1];
	}

	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	bool hr_adaptive_targeting = false;

	if (!adaptive_targeting_arg.compare("on")) {
		hr_adaptive_targeting = true;
	} else if (adaptive_targeting_arg.compare("off")) {
		c->Message(
			Chat::White,
			"Adaptive targeting is currently '%s' for %s's Heal Rotation",
			(((*current_member->MemberOfHealRotation())->AdaptiveTargeting()) ? ("on") : ("off")),
			current_member->GetCleanName());
		return;
	}

	(*current_member->MemberOfHealRotation())->SetAdaptiveTargeting(hr_adaptive_targeting);

	c->Message(
		Chat::White,
		"Adaptive targeting is now '%s' for %s's Heal Rotation",
		(((*current_member->MemberOfHealRotation())->AdaptiveTargeting()) ? ("on") : ("off")),
		current_member->GetCleanName());
}

void bot_command_heal_rotation_add_member(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_heal_rotation_add_member", sep->arg[0], "healrotationaddmember")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s [new_member_name] ([member_name])", sep->arg[0]);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty()) {
		c->Message(Chat::White, "You must [name] a new member as a bot that you own to use this command");
		return;
	}

	auto new_member = sbl.front();
	if (!new_member) {
		c->Message(Chat::White, "Error: New member bot dereferenced to nullptr");
		return;
	}

	if (new_member->IsHealRotationMember()) {
		c->Message(
			Chat::White,
			"%s is already a current member of a Heal Rotation and can not join another one",
			new_member->GetCleanName());
		return;
	}

	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[2]);
	if (sbl.empty()) {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	}
	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	if (!new_member->JoinHealRotationMemberPool(current_member->MemberOfHealRotation())) {
		c->Message(
			Chat::White,
			"Failed to add %s as a current member of this Heal Rotation",
			new_member->GetCleanName());
		return;
	}

	c->Message(
		Chat::White,
		"Successfully added %s as a current member of this Heal Rotation",
		new_member->GetCleanName());
}

void bot_command_heal_rotation_add_target(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_heal_rotation_add_target", sep->arg[0], "healrotationaddtarget")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s [heal_target_name] ([member_name])", sep->arg[0]);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[2]);
	if (sbl.empty()) {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	}
	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	auto heal_target = entity_list.GetMob(sep->arg[1]);
	if (!heal_target) {
		c->Message(Chat::White, "No target exists by the name '%s'", sep->arg[1]);
		return;
	}

	if ((!heal_target->IsClient() && !heal_target->IsBot() && !heal_target->IsPet()) ||
		(heal_target->IsPet() &&
		 (!heal_target->GetOwner() || (!heal_target->GetOwner()->IsClient() && !heal_target->GetOwner()->IsBot())))) {
		c->Message(Chat::White, "%s's entity type is not an allowable heal target", heal_target->GetCleanName());
		return;
	}

	if (!heal_target->JoinHealRotationTargetPool(current_member->MemberOfHealRotation())) {
		c->Message(Chat::White, "Failed to add heal target with a name of '%s'", heal_target->GetCleanName());
		return;
	}

	c->Message(
		Chat::White,
		"Successfully added heal target %s to %s's Heal Rotation",
		heal_target->GetCleanName(),
		current_member->GetCleanName());
}

void bot_command_heal_rotation_adjust_critical(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(
		c,
		"bot_command_heal_rotation_adjust_critical",
		sep->arg[0],
		"healrotationadjustcritical"
	)) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			"usage: (<target_member>) %s [armor_type] [value: %3.1f-%3.1f | + | -] ([member_name])",
			sep->arg[0],
			CRITICAL_HP_RATIO_BASE,
			SAFE_HP_RATIO_BASE
		);
		c->Message(
			Chat::White, "armor_types: %u(Base), %u(Cloth), %u(Leather), %u(Chain), %u(Plate)",
			ARMOR_TYPE_UNKNOWN, ARMOR_TYPE_CLOTH, ARMOR_TYPE_LEATHER, ARMOR_TYPE_CHAIN, ARMOR_TYPE_PLATE
		);
		return;
	}

	std::string armor_type_arg = sep->arg[1];
	std::string critical_arg   = sep->arg[2];

	uint8 armor_type_value = 255;
	if (sep->IsNumber(1)) {
		armor_type_value = Strings::ToInt(armor_type_arg);
	}

	if (armor_type_value > ARMOR_TYPE_LAST) {
		c->Message(
			Chat::White,
			"You must specify a valid [armor_type: %u-%u] to use this command",
			ARMOR_TYPE_FIRST,
			ARMOR_TYPE_LAST
		);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[3]);
	if (sbl.empty()) {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	}
	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	float critical_ratio = CRITICAL_HP_RATIO_BASE;
	if (sep->IsNumber(2)) {
		critical_ratio = Strings::ToFloat(critical_arg);
	} else if (!critical_arg.compare("+")) {
		critical_ratio =
			(*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(armor_type_value) + HP_RATIO_DELTA;
	} else if (!critical_arg.compare("-")) {
		critical_ratio =
			(*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(armor_type_value) - HP_RATIO_DELTA;
	}

	if (critical_ratio > SAFE_HP_RATIO_ABS) {
		critical_ratio = SAFE_HP_RATIO_ABS;
	}
	if (critical_ratio < CRITICAL_HP_RATIO_ABS) {
		critical_ratio = CRITICAL_HP_RATIO_ABS;
	}

	if (!(*current_member->MemberOfHealRotation())->SetArmorTypeCriticalHPRatio(armor_type_value, critical_ratio)) {
		c->Message(
			Chat::White,
			"Critical value %3.1f%%(%u) exceeds safe value %3.1f%%(%u) for %s's Heal Rotation",
			critical_ratio,
			armor_type_value,
			(*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(armor_type_value),
			armor_type_value,
			current_member->GetCleanName());
		return;
	}

	c->Message(
		Chat::White,
		"Class Armor Type %u critical value %3.1f%% set for %s's Heal Rotation",
		armor_type_value,
		(*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(armor_type_value),
		current_member->GetCleanName());
}

void bot_command_heal_rotation_adjust_safe(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_heal_rotation_adjust_safe", sep->arg[0], "healrotationadjustsafe")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			"usage: (<target_member>) %s [armor_type] [value: %3.1f-%3.1f | + | -] ([member_name])",
			sep->arg[0],
			CRITICAL_HP_RATIO_BASE,
			SAFE_HP_RATIO_BASE
		);
		c->Message(
			Chat::White, "armor_types: %u(Base), %u(Cloth), %u(Leather), %u(Chain), %u(Plate)",
			ARMOR_TYPE_UNKNOWN, ARMOR_TYPE_CLOTH, ARMOR_TYPE_LEATHER, ARMOR_TYPE_CHAIN, ARMOR_TYPE_PLATE
		);
		return;
	}

	std::string armor_type_arg = sep->arg[1];
	std::string safe_arg       = sep->arg[2];

	uint8 armor_type_value = 255;
	if (sep->IsNumber(1)) {
		armor_type_value = Strings::ToInt(armor_type_arg);
	}

	if (armor_type_value > ARMOR_TYPE_LAST) {
		c->Message(
			Chat::White,
			"You must specify a valid [armor_type: %u-%u] to use this command",
			ARMOR_TYPE_FIRST,
			ARMOR_TYPE_LAST
		);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[3]);
	if (sbl.empty()) {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	}
	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	float safe_ratio = SAFE_HP_RATIO_BASE;
	if (sep->IsNumber(2)) {
		safe_ratio = Strings::ToFloat(safe_arg);
	} else if (!safe_arg.compare("+")) {
		safe_ratio = (*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(armor_type_value) + HP_RATIO_DELTA;
	} else if (!safe_arg.compare("-")) {
		safe_ratio = (*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(armor_type_value) - HP_RATIO_DELTA;
	}

	if (safe_ratio > SAFE_HP_RATIO_ABS) {
		safe_ratio = SAFE_HP_RATIO_ABS;
	}
	if (safe_ratio < CRITICAL_HP_RATIO_ABS) {
		safe_ratio = CRITICAL_HP_RATIO_ABS;
	}

	if (!(*current_member->MemberOfHealRotation())->SetArmorTypeSafeHPRatio(armor_type_value, safe_ratio)) {
		c->Message(
			Chat::White,
			"Safe value %3.1f%%(%u) does not exceed critical value %3.1f%%(%u) for %s's Heal Rotation",
			safe_ratio,
			armor_type_value,
			(*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(armor_type_value),
			armor_type_value,
			current_member->GetCleanName());
		return;
	}

	c->Message(
		Chat::White,
		"Class Armor Type %u safe value %3.1f%% set for %s's Heal Rotation",
		armor_type_value,
		(*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(armor_type_value),
		current_member->GetCleanName());
}

void bot_command_heal_rotation_casting_override(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(
		c,
		"bot_command_heal_rotation_casting_override",
		sep->arg[0],
		"healrotationcastingoverride"
	)) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s ([member_name]) ([option: on | off])", sep->arg[0]);
		return;
	}

	std::string casting_override_arg;

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (!sbl.empty()) {
		casting_override_arg = sep->arg[2];
	} else {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
		casting_override_arg = sep->arg[1];
	}

	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	bool hr_casting_override = false;

	if (!casting_override_arg.compare("on")) {
		hr_casting_override = true;
	} else if (casting_override_arg.compare("off")) {
		c->Message(
			Chat::White,
			"Casting override is currently '%s' for %s's Heal Rotation",
			(((*current_member->MemberOfHealRotation())->CastingOverride()) ? ("on") : ("off")),
			current_member->GetCleanName());
		return;
	}

	(*current_member->MemberOfHealRotation())->SetCastingOverride(hr_casting_override);

	c->Message(
		Chat::White,
		"Casting override is now '%s' for %s's Heal Rotation",
		(((*current_member->MemberOfHealRotation())->CastingOverride()) ? ("on") : ("off")),
		current_member->GetCleanName());
}

void bot_command_heal_rotation_change_interval(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(
		c,
		"bot_command_heal_rotation_change_interval",
		sep->arg[0],
		"healrotationchangeinterval"
	)) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			"usage: (<target_member>) %s ([member_name]) ([interval=%u: %u-%u(seconds)])",
			sep->arg[0],
			CASTING_CYCLE_DEFAULT_INTERVAL_S,
			CASTING_CYCLE_MINIMUM_INTERVAL_S,
			CASTING_CYCLE_MAXIMUM_INTERVAL_S
		);
		return;
	}

	std::string change_interval_arg;

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (!sbl.empty()) {
		change_interval_arg = sep->arg[2];
	} else {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
		change_interval_arg = sep->arg[1];
	}

	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	uint32 hr_change_interval_s = CASTING_CYCLE_DEFAULT_INTERVAL_S;

	if (!change_interval_arg.empty()) {
		hr_change_interval_s = Strings::ToInt(change_interval_arg);
	} else {
		hr_change_interval_s = (*current_member->MemberOfHealRotation())->IntervalS();
		c->Message(
			Chat::White,
			"Casting interval is currently '%i' second%s for %s's Heal Rotation",
			hr_change_interval_s,
			((hr_change_interval_s == 1) ? ("") : ("s")),
			current_member->GetCleanName());
		return;
	}

	if (hr_change_interval_s < CASTING_CYCLE_MINIMUM_INTERVAL_S ||
		hr_change_interval_s > CASTING_CYCLE_MAXIMUM_INTERVAL_S) {
		hr_change_interval_s = CASTING_CYCLE_DEFAULT_INTERVAL_S;
	}

	(*current_member->MemberOfHealRotation())->SetIntervalS(hr_change_interval_s);

	hr_change_interval_s = (*current_member->MemberOfHealRotation())->IntervalS();
	c->Message(
		Chat::White,
		"Casting interval is now '%i' second%s for %s's Heal Rotation",
		hr_change_interval_s,
		((hr_change_interval_s == 1) ? ("") : ("s")),
		current_member->GetCleanName());
}

void bot_command_heal_rotation_clear_hot(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_heal_rotation_clear_hot", sep->arg[0], "healrotationclearhot")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty()) {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	}
	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	if (!(*current_member->MemberOfHealRotation())->ClearHOTTarget()) {
		c->Message(Chat::White, "Failed to clear %s's Heal Rotation HOT", current_member->GetCleanName());
	}

	c->Message(Chat::White, "Succeeded in clearing %s's Heal Rotation HOT", current_member->GetCleanName());
}

void bot_command_heal_rotation_clear_targets(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(
		c,
		"bot_command_heal_rotation_clear_targets",
		sep->arg[0],
		"healrotationcleartargets"
	)) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty()) {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	}
	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	if (!(*current_member->MemberOfHealRotation())->ClearTargetPool()) {
		c->Message(Chat::White, "Failed to clear all targets from %s's Heal Rotation", current_member->GetCleanName());
		return;
	}

	c->Message(Chat::White, "All targets have been cleared from %s's Heal Rotation", current_member->GetCleanName());
}

void bot_command_heal_rotation_create(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_heal_rotation_create", sep->arg[0], "healrotationcreate")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			"usage: (<target_creator>) %s ([creator_name]) ([interval=%u: %u-%u(seconds)] [fastheals=off: on | off] [adaptivetargeting=off: on | off] [castingoverride=off: on | off])",
			sep->arg[0],
			CASTING_CYCLE_DEFAULT_INTERVAL_S,
			CASTING_CYCLE_MINIMUM_INTERVAL_S,
			CASTING_CYCLE_MAXIMUM_INTERVAL_S
		);
		return;
	}

	std::string interval_arg;
	std::string fast_heals_arg;
	std::string adaptive_targeting_arg;
	std::string casting_override_arg;

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (!sbl.empty()) {
		interval_arg           = sep->arg[2];
		fast_heals_arg         = sep->arg[3];
		adaptive_targeting_arg = sep->arg[4];
		casting_override_arg   = sep->arg[5];
	} else {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
		interval_arg           = sep->arg[1];
		fast_heals_arg         = sep->arg[2];
		adaptive_targeting_arg = sep->arg[3];
		casting_override_arg   = sep->arg[4];
	}

	if (sbl.empty()) {
		c->Message(Chat::White, "You must <target> or [name] a creator as a bot that you own to use this command");
		return;
	}

	auto creator_member = sbl.front();
	if (!creator_member) {
		c->Message(Chat::White, "Error: Creator bot dereferenced to nullptr");
		return;
	}

	if (creator_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is already a current member of a Heal Rotation", creator_member->GetCleanName());
		return;
	}

	uint32 hr_interval_s         = CASTING_CYCLE_DEFAULT_INTERVAL_S;
	bool   hr_fast_heals         = false;
	bool   hr_adaptive_targeting = false;
	bool   hr_casting_override   = false;

	if (!casting_override_arg.compare("on")) {
		hr_casting_override = true;
		if (!adaptive_targeting_arg.compare("on")) {
			hr_adaptive_targeting = true;
		}
		if (!fast_heals_arg.compare("on")) {
			hr_fast_heals = true;
		}
		hr_interval_s       = Strings::ToInt(interval_arg);
	} else if (!casting_override_arg.compare("off")) {
		if (!adaptive_targeting_arg.compare("on")) {
			hr_adaptive_targeting = true;
		}
		if (!fast_heals_arg.compare("on")) {
			hr_fast_heals = true;
		}
		hr_interval_s = Strings::ToInt(interval_arg);
	}

	if (hr_interval_s < CASTING_CYCLE_MINIMUM_INTERVAL_S || hr_interval_s > CASTING_CYCLE_MAXIMUM_INTERVAL_S) {
		hr_interval_s = CASTING_CYCLE_DEFAULT_INTERVAL_S;
	}

	hr_interval_s *= 1000; // convert to milliseconds for Bot/HealRotation constructor

	if (!creator_member->CreateHealRotation(hr_interval_s, hr_fast_heals, hr_adaptive_targeting, hr_casting_override)) {
		c->Message(
			Chat::White,
			"Failed to add %s as a current member to a new Heal Rotation",
			creator_member->GetCleanName());
		return;
	}

	std::list<uint32>      member_list;
	std::list<std::string> target_list;
	bool                   load_flag   = false;
	bool                   member_fail = false;
	bool                   target_fail = false;

	database.botdb.LoadHealRotation(creator_member, member_list, target_list, load_flag, member_fail, target_fail);

	if (!load_flag) {
		c->Message(
			Chat::White,
			"Successfully added %s as a current member to a new Heal Rotation",
			creator_member->GetCleanName());
		return;
	}

	if (!member_fail) {
		MyBots::PopulateSBL_BySpawnedBots(c, sbl);
		for (auto member_iter: member_list) {
			if (!member_iter || member_iter == creator_member->GetBotID()) {
				continue;
			}

			bool      member_found = false;
			for (auto bot_iter: sbl) {
				if (bot_iter->GetBotID() != member_iter) {
					continue;
				}

				if (!bot_iter->JoinHealRotationMemberPool(creator_member->MemberOfHealRotation())) {
					c->Message(Chat::White, "Failed to add member '%s'", bot_iter->GetCleanName());
				}
				member_found = true;

				break;
			}

			if (!member_found) {
				c->Message(Chat::White, "Could not locate member with bot id '%u'", member_iter);
			}
		}
	}

	if (!target_fail) {
		for (auto target_iter: target_list) {
			if (target_iter.empty()) {
				continue;
			}

			auto target_mob = entity_list.GetMob(target_iter.c_str());
			if (!target_mob) {
				c->Message(Chat::White, "Could not locate target '%s'", target_iter.c_str());
				continue;
			}

			if (!target_mob->JoinHealRotationTargetPool(creator_member->MemberOfHealRotation())) {
				c->Message(Chat::White, "Failed to add target '%s'", target_mob->GetCleanName());
			}
		}
	}

	c->Message(Chat::White, "Successfully loaded %s's Heal Rotation", creator_member->GetCleanName());
}

void bot_command_heal_rotation_delete(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_heal_rotation_delete", sep->arg[0], "healrotationdelete")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s ([option: all]) ([member_name])", sep->arg[0]);
		return;
	}

	bool all_flag = false;
	int  name_arg = 1;
	if (!strcasecmp(sep->arg[1], "all")) {
		all_flag = true;
		name_arg = 2;
	}

	if (all_flag) {
		database.botdb.DeleteAllHealRotations(c->CharacterID());
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[name_arg]);
	if (sbl.empty()) {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	}
	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!database.botdb.DeleteHealRotation(current_member->GetBotID())) {
		return;
	}

	c->Message(Chat::White, "Succeeded in deleting %s's heal rotation", current_member->GetCleanName());
}

void bot_command_heal_rotation_fast_heals(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_heal_rotation_fast_heals", sep->arg[0], "healrotationfastheals")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s ([member_name]) ([option: on | off])", sep->arg[0]);
		return;
	}

	std::string fast_heals_arg;

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (!sbl.empty()) {
		fast_heals_arg = sep->arg[2];
	} else {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
		fast_heals_arg = sep->arg[1];
	}

	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	bool hr_fast_heals = false;

	if (!fast_heals_arg.compare("on")) {
		hr_fast_heals = true;
	} else if (fast_heals_arg.compare("off")) {
		c->Message(
			Chat::White,
			"Fast heals are currently '%s' for %s's Heal Rotation",
			(((*current_member->MemberOfHealRotation())->FastHeals()) ? ("on") : ("off")),
			current_member->GetCleanName());
		return;
	}

	(*current_member->MemberOfHealRotation())->SetFastHeals(hr_fast_heals);

	c->Message(
		Chat::White,
		"Fast heals are now '%s' for %s's Heal Rotation",
		(((*current_member->MemberOfHealRotation())->FastHeals()) ? ("on") : ("off")),
		current_member->GetCleanName());
}

void bot_command_heal_rotation_list(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_heal_rotation_list", sep->arg[0], "healrotationlist")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty()) {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	}
	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	c->Message(Chat::White, "Heal Rotation Settings:");

	c->Message(
		Chat::White,
		"Current state: %s",
		(((*current_member->MemberOfHealRotation())->IsActive()) ? ("active") : ("inactive")));
	c->Message(Chat::White, "Casting interval: %i seconds", (*current_member->MemberOfHealRotation())->IntervalS());
	c->Message(
		Chat::White,
		"Fast heals: '%s'",
		(((*current_member->MemberOfHealRotation())->FastHeals()) ? ("on") : ("off")));
	c->Message(
		Chat::White,
		"Adaptive targeting: '%s'",
		(((*current_member->MemberOfHealRotation())->AdaptiveTargeting()) ? ("on") : ("off")));
	c->Message(
		Chat::White,
		"Casting override: '%s'",
		(((*current_member->MemberOfHealRotation())->CastingOverride()) ? ("on") : ("off")));
	c->Message(
		Chat::White,
		"HOT state: %s",
		(((*current_member->MemberOfHealRotation())->IsHOTActive()) ? ("active") : ("inactive")));
	c->Message(
		Chat::White,
		"HOT target: %s",
		(((*current_member->MemberOfHealRotation())->HOTTarget())
			? ((*current_member->MemberOfHealRotation())->HOTTarget()->GetCleanName()) : ("null")));

	c->Message(
		Chat::White, "Base hp limits - critical: %3.1f%%, safe: %3.1f%%",
		(*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(ARMOR_TYPE_UNKNOWN),
		(*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(ARMOR_TYPE_UNKNOWN));
	c->Message(
		Chat::White, "Cloth hp limits - critical: %3.1f%%, safe: %3.1f%%",
		(*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(ARMOR_TYPE_CLOTH),
		(*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(ARMOR_TYPE_CLOTH));
	c->Message(
		Chat::White, "Leather hp limits - critical: %3.1f%%, safe: %3.1f%%",
		(*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(ARMOR_TYPE_LEATHER),
		(*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(ARMOR_TYPE_LEATHER));
	c->Message(
		Chat::White, "Chain hp limits - critical: %3.1f%%, safe: %3.1f%%",
		(*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(ARMOR_TYPE_CHAIN),
		(*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(ARMOR_TYPE_CHAIN));
	c->Message(
		Chat::White, "Plate hp limits - critical: %3.1f%%, safe: %3.1f%%",
		(*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(ARMOR_TYPE_PLATE),
		(*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(ARMOR_TYPE_PLATE));

	c->Message(Chat::White, "Heal Rotation Members:");

	int       member_index = 0;
	auto      member_pool  = (*current_member->MemberOfHealRotation())->MemberList();
	for (auto member_iter: *member_pool) {
		if (!member_iter) {
			continue;
		}

		c->Message(Chat::White, "(%i) %s", (++member_index), member_iter->GetCleanName());
	}
	if (!member_index) {
		c->Message(Chat::White, "(0) None");
	}

	c->Message(Chat::White, "Heal Rotation Targets:");

	int       target_index = 0;
	auto      target_pool  = (*current_member->MemberOfHealRotation())->TargetList();
	for (auto target_iter: *target_pool) {
		if (!target_iter) {
			continue;
		}

		c->Message(Chat::White, "(%i) %s", (++target_index), target_iter->GetCleanName());
	}
	if (!target_index) {
		c->Message(Chat::White, "(0) None");
	}
}

void bot_command_heal_rotation_remove_member(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(
		c,
		"bot_command_heal_rotation_remove_member",
		sep->arg[0],
		"healrotationremovemember"
	)) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty()) {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	}
	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	if (!current_member->LeaveHealRotationMemberPool()) {
		c->Message(Chat::White, "Failed to remove %s from their Heal Rotation", current_member->GetCleanName());
		return;
	}

	c->Message(Chat::White, "%s has been removed from their Heal Rotation", current_member->GetCleanName());
}

void bot_command_heal_rotation_remove_target(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(
		c,
		"bot_command_heal_rotation_remove_target",
		sep->arg[0],
		"healrotationremovetarget"
	)) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s [heal_target_name] ([member_name])", sep->arg[0]);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[2]);
	if (sbl.empty()) {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	}
	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	auto heal_target = entity_list.GetMob(sep->arg[1]);
	if (!heal_target) {
		c->Message(Chat::White, "No target exists by the name '%s'", sep->arg[1]);
		return;
	}

	if (!current_member->MemberOfHealRotation()->get()->IsTargetInPool(heal_target) ||
		!heal_target->LeaveHealRotationTargetPool()) {
		c->Message(Chat::White, "Failed to remove heal target with a name of '%s'", heal_target->GetCleanName());
		return;
	}

	c->Message(
		Chat::White,
		"Successfully removed heal target %s from %s's Heal Rotation",
		heal_target->GetCleanName(),
		current_member->GetCleanName());
}

void bot_command_heal_rotation_reset_limits(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(
		c,
		"bot_command_heal_rotation_reset_limits",
		sep->arg[0],
		"healrotationresetlimits"
	)) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty()) {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	}
	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	(*current_member->MemberOfHealRotation())->ResetArmorTypeHPLimits();

	c->Message(
		Chat::White,
		"Class Armor Type HP limit criteria has been set to default values for %s's Heal Rotation",
		current_member->GetCleanName());
}

void bot_command_heal_rotation_save(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_heal_rotation_save", sep->arg[0], "healrotationsave")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty()) {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	}
	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	bool member_fail = false;
	bool target_fail = false;
	if (!database.botdb.SaveHealRotation(current_member, member_fail, target_fail)) {
		return;
	}
	if (member_fail) {
		c->Message(Chat::White, "Failed to save heal rotation members");
	}
	if (target_fail) {
		c->Message(Chat::White, "Failed to save heal rotation targets");
	}

	c->Message(Chat::White, "Succeeded in saving %s's heal rotation", current_member->GetCleanName());
}

void bot_command_heal_rotation_set_hot(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_heal_rotation_set_hot", sep->arg[0], "healrotationsethot")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s [heal_override_target_name] ([member_name])", sep->arg[0]);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[2]);
	if (sbl.empty()) {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	}
	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	auto hot_target = entity_list.GetMob(sep->arg[1]);
	if (!hot_target) {
		c->Message(Chat::White, "No target exists by the name '%s'", sep->arg[1]);
		return;
	}

	if (!(*current_member->MemberOfHealRotation())->IsTargetInPool(hot_target)) {
		c->Message(
			Chat::White,
			"%s is not a target in %s's Heal Rotation",
			hot_target->GetCleanName(),
			current_member->GetCleanName());
		return;
	}

	if (!(*current_member->MemberOfHealRotation())->SetHOTTarget(hot_target)) {
		c->Message(
			Chat::White,
			"Failed to set %s as the HOT in %s's Heal Rotation",
			hot_target->GetCleanName(),
			current_member->GetCleanName());
		return;
	}

	c->Message(
		Chat::White,
		"Succeeded in setting %s as the HOT in %s's Heal Rotation",
		hot_target->GetCleanName(),
		current_member->GetCleanName());
}

void bot_command_heal_rotation_start(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_heal_rotation_start", sep->arg[0], "healrotationstart")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty()) {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	}
	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	if ((*current_member->MemberOfHealRotation())->IsActive()) {
		c->Message(Chat::White, "%s's Heal Rotation is already active", current_member->GetCleanName());
		return;
	}

	if (!current_member->MemberOfHealRotation()->get()->Start()) {
		c->Message(Chat::White, "Failed to start %s's Heal Rotation", current_member->GetCleanName());
		return;
	}

	c->Message(Chat::White, "%s's Heal Rotation is now active", current_member->GetCleanName());
}

void bot_command_heal_rotation_stop(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_heal_rotation_stop", sep->arg[0], "healrotationstop")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty()) {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	}
	if (sbl.empty()) {
		c->Message(
			Chat::White,
			"You must <target> or [name] a current member as a bot that you own to use this command"
		);
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(Chat::White, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(Chat::White, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	if (!(*current_member->MemberOfHealRotation())->IsActive()) {
		c->Message(Chat::White, "%s's Heal Rotation is already inactive", current_member->GetCleanName());
		return;
	}

	if (!current_member->MemberOfHealRotation()->get()->Stop()) {
		c->Message(Chat::White, "Failed to stop %s's Heal Rotation", current_member->GetCleanName());
		return;
	}

	c->Message(Chat::White, "%s's Heal Rotation is now inactive", current_member->GetCleanName());
}
