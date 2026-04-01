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

void SetHeroModel(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set hero_model [Hero Model]");
		c->Message(Chat::White, "Usage: #set hero_model [Hero Model] [Slot]");
		c->Message(
			Chat::White,
			fmt::format(
				"Example: {}",
				Saylink::Silent("#heromodel 63")
			).c_str()
		);
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	uint32 hero_forge_model = Strings::ToUnsignedInt(sep->arg[2]);

	if (arguments > 2) {
		const uint8 slot = Strings::ToUnsignedInt(sep->arg[3]);
		c->GetTarget()->SendTextureWC(slot, 0, hero_forge_model, 0, 0, 0);
	} else {
		if (!hero_forge_model) {
			c->Message(Chat::White, "Hero's Forge Model must be greater than 0.");
			return;
		}

		// Conversion to simplify the command arguments
		// Hero's Forge model is actually model * 1000 + texture * 100 + wearslot
		// Hero's Forge Model slot 7 is actually for Robes, but it still needs to use wearslot 1 in the packet
		hero_forge_model *= 100;

		for (uint8 slot = 0; slot < 7; slot++) {
			c->GetTarget()->SendTextureWC(slot, 0, (hero_forge_model + slot), 0, 0, 0);
		}
	}
}
