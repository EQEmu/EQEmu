/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "zone/bot_command.h"

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
