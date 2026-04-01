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

void command_nukeitem(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #nukeitem [Item ID] - Removes the specified Item ID from you or your player target's inventory");
		return;
	}

	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	auto item_id = Strings::ToInt(sep->arg[1]);
	auto deleted_count = target->NukeItem(item_id);
	if (deleted_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} ({}) deleted from {}.",
				deleted_count,
				database.CreateItemLink(item_id),
				item_id,
				c->GetTargetDescription(target)
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"Could not find any {} ({}) to delete from {}.",
				database.CreateItemLink(item_id),
				item_id,
				c->GetTargetDescription(target)
			).c_str()
		);
	}
}

