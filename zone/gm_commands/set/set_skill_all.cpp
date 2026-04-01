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

void SetSkillAll(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set skill_all [Skill Level]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint16 skill_level = Strings::ToUnsignedInt(sep->arg[2]);

	for (const auto& s : EQ::skills::GetSkillTypeMap()) {
		if (t->CanHaveSkill(s.first) && t->MaxSkill(s.first)) {
			if (c != t) {
				c->Message(
					Chat::White,
					fmt::format(
						"Setting {} ({}) to {} for {}.",
						s.second,
						s.first,
						skill_level > t->MaxSkill(s.first) ? t->MaxSkill(s.first) : skill_level,
						c->GetTargetDescription(t)
					).c_str()
				);
			}

			t->SetSkill(
				s.first,
				skill_level > t->MaxSkill(s.first) ? t->MaxSkill(s.first) : skill_level
			);
		}
	}
}
