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

void SetAdventurePoints(Client *c, const Seperator *sep)
{
	const uint16 arguments = sep->argnum;
	if (arguments < 3 || !sep->IsNumber(2) || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Usage: #set adventure_points [Theme] [Points]");

		c->Message(Chat::White, "Valid themes are as follows:");

		for (const auto& e : ldon_theme_names) {
			if (e.first != LDoNTheme::Unused) {
				c->Message(
					Chat::White,
					fmt::format(
						"Theme {} | {} ({})",
						e.first,
						e.second.first,
						e.second.second
					).c_str()
				);
			}
		}

		return;
	}

	Client* t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint32 theme_id = Strings::ToUnsignedInt(sep->arg[2]);
	const uint32 points   = Strings::ToUnsignedInt(sep->arg[3]);

	if (!LDoNTheme::IsValid(theme_id)) {
		c->Message(Chat::White, "Valid themes are as follows:");

		for (const auto& e : ldon_theme_names) {
			if (e.first != LDoNTheme::Unused) {
				c->Message(
					Chat::White,
					fmt::format(
						"Theme {} | {} ({})",
						e.first,
						e.second.first,
						e.second.second
					).c_str()
				);
			}
		}

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Set {} Points to {} for {}.",
			LDoNTheme::GetName(theme_id),
			Strings::Commify(points),
			c->GetTargetDescription(t)
		).c_str()
	);

	t->SetLDoNPoints(theme_id, points);
}
