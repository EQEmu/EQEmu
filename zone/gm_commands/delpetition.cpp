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

void command_delpetition(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0 || strcasecmp(sep->arg[1], "*") == 0) {
		c->Message(Chat::White, "Usage: #delpetition (petition number) Type #listpetition for a list");
		return;
	}

	c->Message(Chat::Red, "Attempting to delete petition number: %i", Strings::ToInt(sep->argplus[1]));
	std::string query   = StringFormat("DELETE FROM petitions WHERE petid = %i", Strings::ToInt(sep->argplus[1]));
	auto        results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	LogInfo("Delete petition request from [{}], petition number:", c->GetName(), Strings::ToInt(sep->argplus[1]));

}

