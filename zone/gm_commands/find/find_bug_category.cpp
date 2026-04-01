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

void FindBugCategory(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint32 category_id = Strings::ToUnsignedInt(sep->arg[2]);
		const std::string& category_name = Bug::GetName(category_id);
		if (Strings::EqualFold(category_name, "UNKNOWN BUG CATEGORY")) {
			c->Message(
				Chat::White,
				fmt::format(
					"Bug Category ID {} does not exist.",
					category_id
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Bug Category {} | {}",
				category_id,
				category_name
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (const auto& e : bug_category_names) {
		const std::string& bug_category_name_lower = Strings::ToLower(e.second);
		if (!Strings::Contains(bug_category_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Bug Category {} | {}",
				e.first,
				e.second
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Bug Categor{} found matching '{}'.",
			found_count,
			found_count != 1 ? "ies" : "y",
			sep->argplus[2]
		).c_str()
	);
}
