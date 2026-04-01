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

void command_petname(Client *c, const Seperator *sep)
{
	Mob *t = nullptr;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	if (!t) {
		c->Message(Chat::White, "You must target your pet to use this command.");
		return;
	}

	if (!t->IsPet()) {
		c->Message(Chat::White, "You must target your pet to use this command.");
		return;
	}

	if (t->GetOwnerID() != c->GetID()) {
		c->Message(Chat::White, "You must target your pet to use this command.");
		return;
	}

	if (sep->arg[1]) {
		const std::string& old_name = t->GetCleanName();
		const std::string& new_name = sep->arg[1];

		t->TempName(new_name.c_str());

		c->Message(
			Chat::White,
			fmt::format(
				"Renamed your pet from {} to {}.",
				old_name,
				new_name
			).c_str()
		);

		return;
	}

	t->TempName();
	c->Message(Chat::White, "Restored the original name.");
}
