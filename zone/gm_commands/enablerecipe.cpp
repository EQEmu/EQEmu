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

void command_enablerecipe(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #enablerecipe [Recipe ID]");
		return;
	}

	auto recipe_id = Strings::ToUnsignedInt(sep->arg[1]);
	if (!recipe_id) {
		c->Message(Chat::White, "Usage: #enablerecipe [Recipe ID]");
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Recipe ID {} {} enabled.",
			recipe_id,
			(
				content_db.EnableRecipe(recipe_id) ?
				"successfully" :
				"failed to be"
			)
		).c_str()
	);
}
