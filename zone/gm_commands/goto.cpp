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

void command_goto(Client *c, const Seperator *sep)
{
	const uint16 arguments = sep->argnum;

	const bool goto_player   = arguments > 0 && !sep->IsNumber(1);
	const bool goto_position = sep->IsNumber(1) && sep->IsNumber(2) && sep->IsNumber(3);
	const bool goto_target   = !arguments && c->GetTarget();

	if (!goto_player && !goto_position && !goto_target) {
		c->Message(Chat::White, "Usage: #goto [x y z] [h]");
		c->Message(Chat::White, "Usage: #goto [player_name]");
		c->Message(Chat::White, "Usage: #goto (Target required)");
		return;
	}

	if (goto_player) {
		const std::string& name = sep->arg[1];
		Client* t = entity_list.GetClientByName(name.c_str());
		if (t) {
			c->MovePC(
				zone->GetZoneID(),
				zone->GetInstanceID(),
				t->GetX(),
				t->GetY(),
				t->GetZ(),
				t->GetHeading()
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Going to player {} in the same zone.",
					name
				).c_str()
			);
		} else if (c->GotoPlayer(name)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Going to player {} in a different zone.",
					name
				).c_str()
			);
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"Player {} could not be found.",
					name
				).c_str()
			);
		}
	} else if (goto_position) {
		const glm::vec4& position = glm::vec4(
			Strings::ToFloat(sep->arg[1]),
			Strings::ToFloat(sep->arg[2]),
			Strings::ToFloat(sep->arg[3]),
			sep->arg[4] && Strings::IsFloat(sep->arg[4]) ? Strings::ToFloat(sep->arg[4]) : c->GetHeading()
		);
		c->MovePC(
			zone->GetZoneID(),
			zone->GetInstanceID(),
			position.x,
			position.y,
			position.z,
			position.w
		);
	} else if (goto_target) {
		Mob* t = c->GetTarget();

		c->MovePC(
			zone->GetZoneID(),
			zone->GetInstanceID(),
			t->GetX(),
			t->GetY(),
			t->GetZ(),
			t->GetHeading()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Going to {} in the same zone.",
				c->GetTargetDescription(t)
			).c_str()
		);
	}
}

