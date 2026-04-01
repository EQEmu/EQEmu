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
#include "common/seperator.h"
#include "zone/client.h"

void FindAA(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const auto  aa_id   = Strings::ToInt(sep->arg[2]);
		const auto& aa_name = zone->GetAAName(aa_id);
		if (!aa_name.empty()) {
			c->Message(
				Chat::White,
				fmt::format(
					"AA {} | {}",
					aa_id,
					aa_name
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"AA ID {} was not found.",
				aa_id
			).c_str()
		);

		return;
	}

	const auto& search_criteria = Strings::ToLower(sep->argplus[2]);

	auto found_count = 0;

	std::map<int, std::string> ordered_aas;

	for (const auto &a: zone->aa_abilities) {
		ordered_aas[a.second.get()->first->id] = a.second.get()->name;
	}

	for (const auto &a: ordered_aas) {
		const auto& aa_name = zone->GetAAName(a.first);
		if (!aa_name.empty()) {
			const auto& aa_name_lower = Strings::ToLower(aa_name);
			if (!Strings::Contains(aa_name_lower, search_criteria)) {
				continue;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"AA {} | {}",
					a.first,
					aa_name
				).c_str()
			);

			found_count++;

			if (found_count == 50) {
				break;
			}
		}
	}

	if (found_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 AAs were found matching '{}', max reached.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} AA{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}
