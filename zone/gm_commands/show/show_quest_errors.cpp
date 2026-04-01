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
#include "zone/quest_parser_collection.h"

void ShowQuestErrors(Client *c, const Seperator *sep)
{
	std::list<std::string> l;
	parse->GetErrors(l);

	if (!l.size()) {
		c->Message(Chat::White, "There are no Quest errors currently.");
		return;
	}

	c->Message(Chat::White, "Quest errors currently are as follows:");

	uint32 error_count = 0;

	for (const auto& e : l) {
		if (error_count >= RuleI(World, MaximumQuestErrors)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Maximum of {} error{} shown.",
					RuleI(World, MaximumQuestErrors),
					RuleI(World, MaximumQuestErrors) != 1 ? "s" : ""
				).c_str()
			);
			break;
		}

		c->Message(Chat::White, e.c_str());

		error_count++;
	}
}
