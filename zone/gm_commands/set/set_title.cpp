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
#include "zone/titles.h"

void SetTitle(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set title [Title]");
		c->Message(Chat::White, "Note: Use \"-1\" to remove title.");
		return;
	}

	const bool is_remove = Strings::EqualFold(sep->argplus[2], "-1");
	std::string title = !is_remove ? sep->argplus[2] : "";

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	if (title.size() > 31) {
		c->Message(Chat::White, "Title must be 31 characters or less.");
		return;
	}

	if (!title.empty()) {
		Strings::FindReplace(title, "_", " ");
	}

	if (is_remove) {
		t->SetAATitle(title);
	} else {
		title_manager.CreateNewPlayerTitle(t, title);
	}

	t->Save();

	c->Message(
		Chat::White,
		fmt::format(
			"Title has been {}{} for {}{}",
			is_remove ? "removed" : "changed",
			!is_remove ? " and saved" : "",
			c->GetTargetDescription(t),
			(
				is_remove ?
				"." :
				fmt::format(
					" to '{}'.",
					title
				)
			)
		).c_str()
	);
}
