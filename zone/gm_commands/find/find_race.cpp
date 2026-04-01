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

void FindRace(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const auto race_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[2]));
		const std::string& race_name = GetRaceIDName(race_id);
		if (EQ::ValueWithin(race_id, Race::Human, Race::Pegasus3)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Race {} | {}{}",
					race_id,
					race_name,
					(
						IsPlayerRace(race_id) ?
						fmt::format(
							" ({})",
							Strings::Commify(GetPlayerRaceBit(race_id))
						) :
						""
					)
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Race ID {} was not found.",
				race_id
			).c_str()
		);

		return;
	}

	const auto& search_criteria = Strings::ToLower(sep->argplus[2]);

	auto found_count = 0;

	for (uint16 race_id = Race::Human; race_id <= Race::Pegasus3; race_id++) {
		std::string race_name = GetRaceIDName(race_id);
		auto race_name_lower = Strings::ToLower(race_name);
		if (!Strings::Contains(race_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Race {} | {}{}",
				race_id,
				race_name,
				(
					IsPlayerRace(race_id) ?
					fmt::format(
						" ({})",
						Strings::Commify(GetPlayerRaceBit(race_id))
					) :
					""
				)
			).c_str()
		);

		found_count++;

		if (found_count == 50) {
			break;
		}
	}

	if (found_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 Races found matching '{}', max reached.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Race{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}
