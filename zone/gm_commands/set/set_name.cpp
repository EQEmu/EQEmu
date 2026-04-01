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

void SetName(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set name [Name]");
		return;
	}

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		auto t = c->GetTarget()->CastToClient();

		std::string new_name = sep->arg[2];
		std::string old_name = t->GetCleanName();

		if (t->ChangeFirstName(new_name, c->GetCleanName())) {
			c->Message(
				Chat::White,
				fmt::format(
					"Successfully renamed {} to {}",
					old_name,
					new_name
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Unable to rename {}. Check that the new name '{}' isn't already taken (Including Pet Names), or isn't invalid",
				old_name,
				new_name
			).c_str()
		);
	}
}
