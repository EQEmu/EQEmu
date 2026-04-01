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

void SetAnimation(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set animation [Animation ID]");

		for (const auto& a : EQ::constants::GetSpawnAnimationMap()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Animation {} | {}",
					a.first,
					a.second
				).c_str()
			);
		}

		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const auto animation_id = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
	if (
		!EQ::ValueWithin(
			animation_id,
			static_cast<uint8>(eaStanding),
			static_cast<uint8>(eaLooting)
		)
	) {
		c->Message(Chat::White, "Usage: #set animation [Animation ID]");

		for (const auto& a : EQ::constants::GetSpawnAnimationMap()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Animation {} | {}",
					a.first,
					a.second
				).c_str()
			);
		}

		return;
	}

	t->SetAppearance(static_cast<EmuAppearance>(animation_id), false);

	c->Message(
		Chat::White,
		fmt::format(
			"Set animation to {} ({}) for {}.",
			EQ::constants::GetSpawnAnimationName(animation_id),
			animation_id,
			c->GetTargetDescription(t)
		).c_str()
	);
}
