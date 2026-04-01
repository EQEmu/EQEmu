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

void command_attack(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	std::string entity_name = sep->argplus[1];
	if (entity_name.empty()) {
		c->Message(Chat::White, "Usage: #attack [Entity Name]");
		return;
	}

	auto entity = entity_list.GetMob(entity_name.c_str());
	if (entity) {
		c->GetTarget()->AddToHateList(entity, 1);
		c->Message(
			Chat::EchoChat1,
			fmt::format(
				"{} whispers, 'Attacking {}.'",
				c->GetTarget()->GetCleanName(),
				c->GetTargetDescription(entity)
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"No entity by the name of '{}' could be found.",
				entity_name
			).c_str()
		);
	}
}

