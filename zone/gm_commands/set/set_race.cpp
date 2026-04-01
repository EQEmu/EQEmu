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

void SetRace(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: #set race [0-{}, 2253-2259] (0 for back to normal)",
				RuleI(NPC, MaxRaceID)
			).c_str()
		);

		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const uint16 race_id = Strings::ToUnsignedInt(sep->arg[2]);

	if (
		!EQ::ValueWithin(race_id, Race::Doug, RuleI(NPC, MaxRaceID)) &&
		!EQ::ValueWithin(race_id, 2253, 2259)
	) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: #race [0-{}, 2253-2259] (0 for back to normal)",
				RuleI(NPC, MaxRaceID)
			).c_str()
		);

		return;
	}

	t->SendIllusionPacket(
		AppearanceStruct{
			.race_id = race_id,
			.size = t->GetSize(),
		}
	);

	if (race_id == Race::Doug) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} been returned to {} base race.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "have" : "has",
				c == t ? "your" : "their"
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} {} now temporarily a(n) {} ({}).",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "are" : "is",
			GetRaceIDName(race_id),
			race_id
		).c_str()
	);
}
