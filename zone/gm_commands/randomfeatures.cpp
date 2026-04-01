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

void command_randomfeatures(Client *c, const Seperator *sep)
{
	if (!c->GetTarget()) {
		c->Message(Chat::White, "You must have a target to use this command.");
		return;
	}

	auto target = c->GetTarget();

	if (target->RandomizeFeatures()) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} had {} features randomized.",
				c->GetTargetDescription(target, TargetDescriptionType::UCYou),
				c == target ? "have" : "had",
				c == target ? "your" : "their"
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} not a player race, {} race is {} ({}).",
				c->GetTargetDescription(target, TargetDescriptionType::UCYou),
				c == target ? "are" : "is",
				c == target ? "your" : "their",
				GetRaceIDName(target->GetRace()),
				target->GetRace()
			).c_str()
		);
	}
}

