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

void FindStance(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint8 stance_id = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		const std::string& stance_name = Stance::GetName(stance_id);
		if (Strings::EqualFold(stance_name, "UNKNOWN STANCE")) {
			c->Message(
				Chat::White,
				fmt::format(
					"Stance ID {} does not exist.",
					stance_id
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Stance {} | {}",
				stance_id,
				stance_name
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (const auto& e : stance_names) {
		const std::string& stance_name_lower = Strings::ToLower(e.second);
		if (!Strings::Contains(stance_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Stance {} | {}",
				e.first,
				e.second
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Stance{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}
