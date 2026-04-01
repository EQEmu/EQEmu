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
#include "zone/groups.h"

void command_refreshgroup(Client *c, const Seperator *sep)
{
	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	Group *group = target->GetGroup();

	if (!group) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} not in a group.",
				c->GetTargetDescription(target, TargetDescriptionType::UCYou),
				c == target ? "are" : "is"
			).c_str()
		);
		return;
	}

	database.RefreshGroupFromDB(target);

	c->Message(
		Chat::White,
		fmt::format(
			"Group has been refreshed for {}.",
			c->GetTargetDescription(target)
		).c_str()
	);

	if (c != target) {
		target->Message(
			Chat::White,
			fmt::format(
				"Your group has been refreshed by {}.",
				c->GetCleanName()
			).c_str()
		);
	}
}

