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

void command_disarmtrap(Client *c, const Seperator *sep)
{
	Mob *t = c->GetTarget();
	if (!t || !t->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	if (!c->HasSkill(EQ::skills::SkillDisarmTraps)) {
		c->Message(Chat::White, "You do not have the Disarm Trap skill.");
		return;
	}

	if (DistanceSquaredNoZ(c->GetPosition(), t->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse)) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} is too far away.",
				t->GetCleanName()
			).c_str()
		);
		return;
	}

	c->HandleLDoNDisarm(t->CastToNPC(), c->GetSkill(EQ::skills::SkillDisarmTraps), LDoNTypeMechanical);
}
