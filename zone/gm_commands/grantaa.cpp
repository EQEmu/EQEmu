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

void command_grantaa(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsClient()) {
		c->Message(Chat::White, "You must target a player to use this command.");
		return;
	}

	const uint8 unlock_level    = sep->IsNumber(1) ? static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[1])) : 0;
	const bool  skip_grant_only = sep->IsNumber(2) ? Strings::ToBool(sep->arg[2]) : false;

	auto t = c->GetTarget()->CastToClient();
	t->GrantAllAAPoints(unlock_level, skip_grant_only);

	c->Message(
		Chat::White,
		fmt::format(
			"Successfully granted all Alternate Advancements for {}{}{}.",
			c->GetTargetDescription(t),
			(
				unlock_level ?
				fmt::format(
					" up to level {}",
					unlock_level
				) :
				""
			),
			skip_grant_only ? "except for grant only AAs" : ""
		).c_str()
	);
}
