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

void command_spawn(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #spawn [Name]");
		c->Message(
			Chat::White,
			"Optional Usage: #spawn [Name] [Race] [Level] [Texture] [Health] [Gender] [Class] [Primary Model] [Secondary Model] [Merchant ID] [Body Type]"
		);
		c->Message(
			Chat::White,
			"Name Format: NPCFirstname_NPCLastname - All numbers in a name are stripped and \"_\" characters become a space."
		);
		c->Message(
			Chat::White,
			"Note: Using \"-\" for gender will autoselect the gender for the race. Using \"-\" for HP will use the calculated maximum HP."
		);
		return;
	}

	const auto* m = entity_list.GetClientByName(sep->arg[1]);
	if (m) {
		c->Message(Chat::White, "You cannot spawn a mob with the same name as a character!");
		return;
	}

	const auto* n = NPC::SpawnNPC(sep->argplus[1], c->GetPosition(), c);
	if (!n) {
		c->Message(Chat::White, "Usage: #spawn [Name]");
		c->Message(
			Chat::White,
			"Optional Usage: #spawn [Name] [Race] [Level] [Texture] [Health] [Gender] [Class] [Primary Model] [Secondary Model] [Merchant ID] [Body Type]"
		);
		c->Message(
			Chat::White,
			"Name Format: NPCFirstname_NPCLastname - All numbers in a name are stripped and \"_\" characters become a space."
		);
		c->Message(
			Chat::White,
			"Note: Using \"-\" for gender will autoselect the gender for the race. Using \"-\" for HP will use the calculated maximum HP."
		);
	}
}

