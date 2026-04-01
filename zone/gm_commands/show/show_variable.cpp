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

void ShowVariable(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #show variable [Variable Name]");
		return;
	}

	const std::string& variable = sep->argplus[2];

	std::string value;
	if (!database.GetVariable(variable, value)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Variable '{}' was not found.",
				variable
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Variable {} | {}",
			variable,
			value
		).c_str()
	);
}
