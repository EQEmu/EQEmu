#include "../bot_command.h"

void bot_command_bot_settings(Client* c, const Seperator* sep)
{
	std::vector<const char*> subcommand_list = {
		"behindmob",
		"blockedbuffs",
		"blockedpetbuffs",
		"distanceranged",
		"copysettings",
		"defaultsettings",
		"enforcespelllist",
		"follow",
		"followdistance",
		"illusionblock",
		"maxmeleerange",
		"owneroption",
		"petsettype",
		"sithppercent",
		"sitincombat",
		"sitmanapercent",
		"spellaggrochecks",
		"spellannouncecasts",
		"spelldelays",
		"spellengagedpriority",
		"spellholds",
		"spellidlepriority",
		"spellmaxhppct",
		"spellmaxmanapct",
		"spellmaxthresholds",
		"spellminhppct",
		"spellminmanapct",
		"spellminthresholds",
		"spellpursuepriority",
		"spellresistlimits",
		"spelltargetcount",
		"spelllist",
		"stance",
		"togglehelm",
		"bottoggleranged"
	};

	if (helper_command_alias_fail(c, "bot_command_bot_settings", sep->arg[0], "botsettings"))
		return;

	helper_send_available_subcommands(c, "botsettings", subcommand_list);
}
