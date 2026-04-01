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
#include "zone/client.h"

void command_traindisc(Client *c, const Seperator *sep)
{
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		target = c->GetTarget()->CastToClient();
	}

	if (sep->argnum < 1 || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #traindisc [Max Level] [Min Level]");
		return;
	}

	uint8 rule_max_level = (uint8) RuleI(Character, MaxLevel);
	uint8 max_level      = (uint8) Strings::ToInt(sep->arg[1]);
	uint8 min_level      = (
		sep->IsNumber(2) ?
			(uint8)
				Strings::ToInt(sep->arg[2]) :
			1
	); // Default to Level 1 if there isn't a 2nd argument

	if (!c->GetGM()) { // Default to Character:MaxLevel if we're not a GM and Level is higher than the max level
		if (max_level > rule_max_level) {
			max_level = rule_max_level;
		}

		if (min_level > rule_max_level) {
			min_level = rule_max_level;
		}
	}

	if (max_level < 1 || min_level < 1) {
		c->Message(Chat::White, "Level must be greater than or equal to 1.");
		return;
	}

	if (min_level > max_level) {
		c->Message(Chat::White, "Minimum Level must be less than or equal to Maximum Level.");
		return;
	}

	uint16 learned_disciplines = target->LearnDisciplines(min_level, max_level);
	if (c != target) {
		std::string discipline_message = (
			learned_disciplines > 0 ?
			(
				learned_disciplines == 1 ?
				"A new discipline" :
				fmt::format(
					"{} New disciplines",
					learned_disciplines
				)
			) :
			"No new disciplines"
		);
		c->Message(
			Chat::White,
			fmt::format(
				"{} learned for {}.",
				discipline_message,
				c->GetTargetDescription(target)
			).c_str()
		);
	}
}

