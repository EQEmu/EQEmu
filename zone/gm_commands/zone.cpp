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
#include "zone/bot.h"
#include "zone/client.h"

void command_zone(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #zone [Zone ID|Zone Short Name] [X] [Y] [Z]");
		return;
	}

	// input: (first arg)
	// zone identifier can be a string of a zone name or its ID
	std::string zone_input = sep->arg[1];
	uint32      zone_id    = 0;

	// if input is id
	if (Strings::IsNumber(zone_input)) {
		zone_id = Strings::ToInt(zone_input);

		// validate
		if (zone_id != 0 && !GetZone(zone_id)) {
			c->Message(Chat::White, fmt::format("Could not find zone by id [{}]", zone_id).c_str());
			return;
		}
	}
	else {
		// validate
		if (!ZoneStore::Instance()->GetZone(zone_input)) {
			c->Message(Chat::White, fmt::format("Could not find zone by short_name [{}]", zone_input).c_str());
			return;
		}

		// validate we got id
		zone_id = ZoneID(zone_input);
		if (zone_id == 0) {
			c->Message(Chat::White, fmt::format("Could not find zone id by short_name [{}]", zone_input).c_str());
			return;
		}
	}

	// if zone identifier is a number and the id is 0, send to safe coordinates of the local zone
	if (Strings::IsNumber(zone_input) && zone_id == 0) {
		c->Message(Chat::White, "Sending you to the safe coordinates of this zone.");

		c->MovePC(
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			0,
			ZoneToSafeCoords
		);
		return;
	}

	// This block is necessary to clean up any bot objects owned by a Client
	if (RuleB(Bots, Enabled) && zone_id != c->GetZoneID()) {
		Bot::ProcessClientZoneChange(c);
	}

	// fetch zone data
	auto zd = GetZoneVersionWithFallback(zone_id, 0);
	if (zone_id == 0) {
		c->Message(Chat::White, fmt::format("Failed to find zone with fallback [{}]", zone_id).c_str());
		return;
	}

	// coordinates
	auto x         = sep->IsNumber(2) ? Strings::ToFloat(sep->arg[2]) : 0.0f;
	auto y         = sep->IsNumber(3) ? Strings::ToFloat(sep->arg[3]) : 0.0f;
	auto z         = sep->IsNumber(4) ? Strings::ToFloat(sep->arg[4]) : 0.0f;
	auto zone_mode = sep->IsNumber(2) ? ZoneSolicited : ZoneToSafeCoords;

	c->MovePC(
		zone_id,
		x,
		y,
		z,
		zd ? zd->safe_heading : 0.0f,
		0,
		zone_mode
	);
}
