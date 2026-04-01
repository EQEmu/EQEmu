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

void command_countitem(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #countitem [Item ID]");
		return;
	}

	Mob* target = c;
	if (
		c->GetTarget() &&
		(
			c->GetTarget()->IsClient() ||
			c->GetTarget()->IsNPC()
		)
	) {
		target = c->GetTarget();
	}

	auto item_id = Strings::ToUnsignedInt(sep->arg[1]);
	if (!database.GetItem(item_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Item ID {} could not be found.",
				item_id
			).c_str()
		);
		return;
	}

	uint16 item_count = 0;
	if (target->IsClient()) {
		item_count = target->CastToClient()->CountItem(item_id);
	} else if (target->IsNPC()) {
		item_count = target->CastToNPC()->CountItem(item_id);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} {} {} {}.",
			c->GetTargetDescription(target, TargetDescriptionType::UCYou),
			c == target ? "have" : "has",
			(
				item_count ?
				std::to_string(item_count) :
				"no"
			),
			database.CreateItemLink(item_id)
		).c_str()
	);
}

