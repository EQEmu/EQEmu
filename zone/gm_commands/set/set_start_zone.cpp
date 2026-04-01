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

void SetStartZone(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set start_zone [Zone ID|Zone Short Name]");
		c->Message(
			Chat::White,
			"Optional Usage: Use '#set start_zone reset' or '#set start_zone 0' to clear a starting zone. Player can select a starting zone using /setstartcity"
		);
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint32 zone_id = (
		sep->IsNumber(2) ?
		Strings::ToUnsignedInt(sep->arg[2]) :
		ZoneID(sep->arg[2])
	);

	t->SetStartZone(zone_id);

	const bool is_reset = (
		Strings::EqualFold(sep->arg[2], "reset") ||
		zone_id == 0
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Start Zone {} for {} |{}",
			is_reset ? "Reset" : "Changed",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf),
			(
				zone_id ?
				fmt::format(
					" {} ({}) ID: {}",
					ZoneLongName(zone_id),
					ZoneName(zone_id),
					zone_id
				) :
				""
			)
		).c_str()
	);
}
