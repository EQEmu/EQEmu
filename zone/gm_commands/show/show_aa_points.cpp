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
#include "zone/dialogue_window.h"

void ShowAAPoints(Client *c, const Seperator *sep)
{
	Client *t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const int aa_points       = t->GetAAPoints();
	const int spent_aa_points = t->GetSpentAA();
	const int total_aa_points = (aa_points + spent_aa_points);

	if (!total_aa_points) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} no AA Points.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "have" : "has"
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"AA Points for {} | Current: {} Spent: {} Total: {}",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf),
			Strings::Commify(aa_points),
			Strings::Commify(spent_aa_points),
			Strings::Commify(total_aa_points)
		).c_str()
	);
}
