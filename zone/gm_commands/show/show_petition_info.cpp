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
#include "common/repositories/petitions_repository.h"

void ShowPetitionInfo(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(2)) {
		const auto& l = PetitionsRepository::All(database);

		uint32 found_count = 0;

		for (const auto& e : l) {
			c->Message(
				Chat::White,
				fmt::format(
					"Petition {} | Name: {} Text: {} Account: {} Zone: {} Class: {} Race: {} Level: {}",
					e.petid,
					e.charname,
					e.petitiontext,
					e.accountname,
					e.zone,
					GetClassIDName(static_cast<uint8>(e.charclass)),
					GetRaceIDName(static_cast<uint16>(e.charrace)),
					e.charlevel
				).c_str()
			);

			found_count++;

			if (found_count == 50) {
				break;
			}
		}

		if (found_count == 50) {
			c->Message(Chat::White, "50 Petitions found, max reached.");
		}

		c->Message(
			Chat::White,
			fmt::format(
				"{} Petition{} found.",
				found_count,
				found_count != 1 ? "s" : ""
			).c_str()
		);

		return;
	}

	const uint32 petition_id = Strings::ToUnsignedInt(sep->arg[2]);

	const auto& l = PetitionsRepository::GetWhere(database, fmt::format("petid = {}", petition_id));
	if (l.empty()) {
		c->Message(
			Chat::White,
			fmt::format(
				"Petition ID {} was not found.",
				petition_id
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Petition {} | Name: {} Text: {} Account: {} Zone: {} Class: {} Race: {} Level: {}",
			l[0].petid,
			l[0].charname,
			l[0].petitiontext,
			l[0].accountname,
			l[0].zone,
			GetClassIDName(static_cast<uint8>(l[0].charclass)),
			GetRaceIDName(static_cast<uint16>(l[0].charrace)),
			l[0].charlevel
		).c_str()
	);
}
