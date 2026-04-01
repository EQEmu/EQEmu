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

void bot_command_track(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_track", sep->arg[0], "track"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s (Ranger: [option=all: all | rare | local])", sep->arg[0]);
		c->Message(Chat::White, "requires one of the following bot classes:");
		c->Message(Chat::White, "Ranger(1), Druid(20) or Bard(35)");
		return;
	}

	std::string tracking_scope = sep->arg[1];

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	uint16 class_mask = (player_class_bitmasks[Class::Ranger] | player_class_bitmasks[Class::Druid] | player_class_bitmasks[Class::Bard]);
	ActionableBots::Filter_ByClasses(c, sbl, class_mask);

	Bot* my_bot = ActionableBots::AsSpawned_ByMinLevelAndClass(c, sbl, 1, Class::Ranger);
	if (tracking_scope.empty()) {
		if (!my_bot)
			my_bot = ActionableBots::AsSpawned_ByMinLevelAndClass(c, sbl, 20, Class::Druid);
		if (!my_bot)
			my_bot = ActionableBots::AsSpawned_ByMinLevelAndClass(c, sbl, 35, Class::Bard);
	}
	if (!my_bot) {
		c->Message(Chat::White, "No bots are capable of performing this action");
		return;
	}

	int base_distance = 0;
	bool track_named = false;
	std::string tracking_msg;
	switch (my_bot->GetClass()) {
		case Class::Ranger:
			if (!tracking_scope.compare("local")) {
				base_distance = 30;
				tracking_msg = "Local tracking...";
			}
			else if (!tracking_scope.compare("rare")) {
				base_distance = 80;
				bool track_named = true;
				tracking_msg = "Master tracking...";
			}
			else { // default to 'all'
				base_distance = 80;
				tracking_msg = "Advanced tracking...";
			}
			break;
		case Class::Druid:
			base_distance = 30;
			tracking_msg = "Local tracking...";
			break;
		case Class::Bard:
			base_distance = 20;
			tracking_msg = "Near tracking...";
			break;
		default:
			return;
	}
	if (!base_distance) {
		c->Message(Chat::White, "An unknown codition has occurred");
		return;
	}

	my_bot->InterruptSpell();
	my_bot->RaidGroupSay(tracking_msg.c_str());
	entity_list.ShowSpawnWindow(c, (c->GetLevel() * base_distance), track_named);
}
