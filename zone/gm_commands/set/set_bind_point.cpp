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

void SetBindPoint(Client *c, const Seperator *sep)
{
	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const bool bind_allowed = (
		!zone->GetInstanceID() ||
		(
			zone->GetInstanceID() &&
			zone->IsInstancePersistent()
		)
	);

	if (!bind_allowed) {
		c->Message(Chat::White, "You cannot bind here.");
		return;
	}

	t->SetBindPoint();

	c->Message(
		Chat::White,
		fmt::format(
			"Set Bind Point for {} | Zone: {}",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf),
			zone->GetZoneDescription()
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Set Bind Point for {} | XYZH: {:.2f}, {:.2f}, {:.2f}, {:.2f}",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf),
			t->GetX(),
			t->GetY(),
			t->GetZ(),
			t->GetHeading()
		).c_str()
	);
}
