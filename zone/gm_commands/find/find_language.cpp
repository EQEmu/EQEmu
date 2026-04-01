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

void FindLanguage(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const auto language_id = Strings::ToInt(sep->arg[2]);
		if (EQ::ValueWithin(language_id, Language::CommonTongue, Language::Unknown27)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Language {} | {}",
					language_id,
					EQ::constants::GetLanguageName(language_id)
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Language ID {} was not found.",
				language_id
			).c_str()
		);

		return;
	}

	const auto& search_criteria = Strings::ToLower(sep->argplus[2]);

	auto found_count = 0;

	for (const auto& l : EQ::constants::GetLanguageMap()) {
		const auto& language_name_lower = Strings::ToLower(l.second);
		if (!Strings::Contains(language_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Language {} | {}",
				l.first,
				l.second
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Language{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}

