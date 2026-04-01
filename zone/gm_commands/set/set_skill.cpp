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

void SetSkill(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 3 || !sep->IsNumber(2) || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Usage: #set skill [Skill ID] [Skill Value]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint16 skill_id    = Strings::ToUnsignedInt(sep->arg[2]);
	const uint16 skill_value = Strings::ToUnsignedInt(sep->arg[3]);

	if (!EQ::ValueWithin(skill_id, EQ::skills::Skill1HBlunt, EQ::skills::HIGHEST_SKILL)) {
		c->Message(Chat::White, "Usage: #set skill [Skill ID] [Skill Value]");
		c->Message(Chat::White, fmt::format("Skill ID: 0 to {}", EQ::skills::HIGHEST_SKILL).c_str());
		return;
	}

	const auto skill_type = static_cast<EQ::skills::SkillType>(skill_id);

	t->SetSkill(
		skill_type,
		skill_value > t->MaxSkill(skill_type) ? t->MaxSkill(skill_type) : skill_value
	);

	if (c != t) {
		c->Message(
			Chat::White,
			fmt::format(
				"Set {} ({}) to {} for {}.",
				EQ::skills::GetSkillName(skill_type),
				skill_id,
				skill_value > t->MaxSkill(skill_type) ? t->MaxSkill(skill_type) : skill_value,
				c->GetTargetDescription(t)
			).c_str()
		);
	}
}
