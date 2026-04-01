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

void SetLastName(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set lastname [Last Name]");
		c->Message(Chat::White, "Note: Use \"-1\" to remove last name.");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	LogInfo("#lastname request from [{}] for [{}]", c->GetCleanName(), t->GetCleanName());

	const bool is_remove = Strings::EqualFold(sep->argplus[2], "-1");
	const std::string& last_name = !is_remove ? sep->argplus[2] : "";

	if (last_name.size() > 64) {
		c->Message(Chat::White, "Last name must be 64 characters or less.");
		return;
	}

	t->ChangeLastName(last_name);

	c->Message(
		Chat::White,
		fmt::format(
			"Last name has been {}{} for {}{}",
			is_remove ? "removed" : "changed",
			!is_remove ? " and saved" : "",
			c->GetTargetDescription(t),
			(
				is_remove ?
				"." :
				fmt::format(
					" to '{}'.",
					last_name
				)
			)
		).c_str()
	);
}
