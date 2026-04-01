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

void command_appearance(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1) || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #appearance [Type] [Value]");
		c->Message(Chat::White, "Note: Types are as follows:");

		for (const auto& a : EQ::constants::GetAppearanceTypeMap()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Appearance Type {} | {}",
					a.first,
					a.second
				).c_str()
			);
		}

		return;
	}

	Mob *t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const uint32 type  = Strings::ToUnsignedInt(sep->arg[1]);
	const uint32 value = Strings::ToUnsignedInt(sep->arg[2]);

	t->SendAppearancePacket(type, value);

	c->Message(
		Chat::White,
		fmt::format(
			"Appearance Sent to {} | Type: {} ({}) Value: {}",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf),
			EQ::constants::GetAppearanceTypeName(type),
			type,
			value
		).c_str()
	);
}
