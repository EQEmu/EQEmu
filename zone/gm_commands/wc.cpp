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
#include "common/data_verification.h"
#include "zone/client.h"

void command_wc(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (
		arguments < 2 ||
		!sep->IsNumber(1) ||
		!sep->IsNumber(2)
	) {
		c->Message(Chat::White, "Usage: #wc [Slot ID] [Material]");
		c->Message(Chat::White, "Usage: #wc [Slot ID] [Material] [Hero Forge Model] [Elite Material]");
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const uint8  slot_id          = Strings::ToUnsignedInt(sep->arg[1]);
	const uint32 texture          = Strings::ToUnsignedInt(sep->arg[2]);
	uint32       hero_forge_model = 0;
	uint32       elite_material   = 0;

	if (arguments >= 3 && sep->IsNumber(3)) {
		hero_forge_model = Strings::ToUnsignedInt(sep->arg[3]);

		if (EQ::ValueWithin(hero_forge_model, 1, 999)) { // Shorthand Hero Forge ID. Otherwise use the value the user entered.
			hero_forge_model = (hero_forge_model * 100) + slot_id;
		}
	}

	if (arguments >= 4 && sep->IsNumber(4)) {
		elite_material = Strings::ToUnsignedInt(sep->arg[4]);
	}

	t->SendTextureWC(
		slot_id,
		texture,
		hero_forge_model,
		elite_material
	);
}
