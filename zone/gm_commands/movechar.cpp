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

void command_movechar(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #movechar [Character ID|Character Name] [Zone ID|Zone Short Name]");
		return;
	}

	const std::string &character_name = (
		sep->IsNumber(1) ?
		database.GetCharNameByID(Strings::ToUnsignedInt(sep->arg[1])) :
		sep->arg[1]
	);
	const uint32 character_id = database.GetCharacterID(character_name);
	if (!character_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Character {} could not be found.",
				character_name
			).c_str()
		);
		return;
	}

	const uint32 account_id = database.GetAccountIDByChar(character_name.c_str());

	const std::string &zone_short_name = Strings::ToLower(
		sep->IsNumber(2) ?
		ZoneName(Strings::ToUnsignedInt(sep->arg[2]), true) :
		sep->arg[2]
	);

	const bool is_unknown_zone = zone_short_name.find("unknown") != std::string::npos;
	if (is_unknown_zone) {
		c->Message(
			Chat::White,
			fmt::format(
				"Zone ID {} could not be found.",
				Strings::ToUnsignedInt(sep->arg[2])
			).c_str()
		);
		return;
	}

	const uint32 zone_id = ZoneID(zone_short_name);
	auto         z       = GetZone(zone_id);

	if (!z) {
		c->Message(Chat::Red, "Invalid zone.");
		return;
	}

	const bool  moved        = database.MoveCharacterToZone(character_name, zone_id);
	std::string moved_string = moved ? "Succeeded" : "Failed";
	c->Message(
		Chat::White,
		fmt::format(
			"Character Move {} | Character: {} ({})",
			moved_string,
			character_name,
			character_id
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Character Move {} | Zone: {} ({}) ID: {}",
			moved_string,
			z->long_name,
			zone_short_name,
			zone_id
		).c_str()
	);
}
