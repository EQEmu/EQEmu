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

void SetTemporaryName(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set temporary_name [New Name]");
		c->Message(Chat::White, "Note: This command requires a target, even if the target is yourself.");
		return;
	}

	if (!c->GetTarget()) {
		c->Message(Chat::White, "Usage: #set temporary_name [New Name]");
		c->Message(Chat::White, "Note: This command requires a target, even if the target is yourself.");
		return;
	}

	Mob* t = c->GetTarget();

	const std::string& temporary_name = strlen(sep->arg[2]) ? sep->arg[2] : "";

	t->TempName(temporary_name.c_str());

	if (temporary_name.length()) {
		const std::string& current_name   = t->GetName();

		c->Message(
			Chat::White,
			fmt::format(
				"Renamed {} to {} temporarily.",
				current_name,
				temporary_name
			).c_str()
		);
	} else {
		c->Message(Chat::White, "Restored your target's original name.");
	}
}
