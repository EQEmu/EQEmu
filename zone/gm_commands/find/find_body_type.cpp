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

void FindBodyType(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint8 body_type_id = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		const std::string& body_type_name = BodyType::GetName(body_type_id);
		if (Strings::EqualFold(body_type_name, "UNKNOWN BODY TYPE")) {
			c->Message(
				Chat::White,
				fmt::format(
					"Body Type {} does not exist.",
					body_type_id
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Body Type {} | {}",
				body_type_id,
				body_type_name
			).c_str()
		);
		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (const auto& e : body_type_names) {
		const std::string& body_type_name_lower = Strings::ToLower(e.second);
		if (!Strings::Contains(body_type_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Body Type {} | {}",
				e.first,
				e.second
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Body Type{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}
