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
#include "zone/bot_command.h"

void bot_command_view_combos(Client *c, const Seperator *sep)
{
	const std::string class_substrs[17] = {
		"",
		"WAR", "CLR", "PAL", "RNG",
		"SHD", "DRU", "MNK", "BRD",
		"ROG", "SHM", "NEC", "WIZ",
		"MAG", "ENC", "BST", "BER"
	};

	const std::string race_substrs[17] = {
		"",
		"HUM", "BAR", "ERU", "ELF",
		"HIE", "DEF", "HEF", "DWF",
		"TRL", "OGR", "HFL", "GNM",
		"IKS", "VAH", "FRG", "DRK"
	};

	const uint16 race_values[17] = {
		Race::Doug,
		Race::Human, Race::Barbarian, Race::Erudite, Race::WoodElf,
		Race::HighElf, Race::DarkElf, Race::HalfElf, Race::Dwarf,
		Race::Troll, Race::Ogre, Race::Halfling, Race::Gnome,
		Race::Iksar, Race::VahShir, Race::Froglok2, Race::Drakkin
	};

	if (helper_command_alias_fail(c, "bot_command_view_combos", sep->arg[0], "viewcombos")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		std::string window_text;
		std::string message_separator = " ";
		c->Message(Chat::White, fmt::format("Usage: {} [Race]", sep->arg[0]).c_str());

		window_text.append("<c \"#FFFF\">");

		for (int race_id = 0; race_id <= 15; ++race_id) {
			window_text.append(message_separator);
			window_text.append(
				fmt::format(
					"{} ({})",
					race_substrs[race_id + 1],
					race_values[race_id + 1]
				)
			);

			message_separator = ", ";
		}
		c->SendPopupToClient("Bot Races", window_text.c_str());
		return;
	}

	if (sep->arg[1][0] == '\0' || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Invalid Race!");
		return;
	}

	const uint16 bot_race = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));
	const std::string race_name = GetRaceIDName(bot_race);

	if (!IsPlayerRace(bot_race)) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} ({}) is not a race bots can use.",
				race_name,
				bot_race
			).c_str()
		);
		return;
	}

	const auto classes_bitmask = database.botdb.GetRaceClassBitmask(bot_race);

	std::string window_text;
	std::string message_separator = " ";

	window_text.append("<c \"#FFFF\">");

	const int object_max = 4;
	auto object_count = 0;

	for (int class_id = 0; class_id <= 15; ++class_id) {
		if (classes_bitmask & GetPlayerClassBit(class_id)) {
			window_text.append(message_separator);

			if (object_count >= object_max) {
				window_text.append(DialogueWindow::Break());
				object_count = 0;
			}

			window_text.append(
				fmt::format(
					"{} ({})",
					class_substrs[class_id],
					class_id
				)
			);

			++object_count;
			message_separator = ", ";
		}
	}

	c->SendPopupToClient(
		fmt::format(
			"Bot Classes for {} ({})",
			race_name,
			bot_race
		).c_str(),
		window_text.c_str()
	);
}
