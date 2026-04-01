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
#include "zone/bot.h"
#include "zone/client.h"

void SetLevel(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set level [Level]");
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const uint8 max_level = RuleI(Character, MaxLevel);
	const uint8 level     = Strings::ToUnsignedInt(sep->arg[2]);

	if (c != t && c->Admin() < RuleI(GM, MinStatusToLevelTarget)) {
		c->Message(Chat::White, "Your status is not high enough to change another person's level.");
		return;
	}

	t->SetLevel(level, true);

	if (t->IsClient()) {
		for (const auto& s : EQ::skills::GetSkillTypeMap()) {
			const uint16 max_skill_value = t->CastToClient()->MaxSkill(s.first);
			if (t->GetSkill(s.first) > max_skill_value) {
				t->CastToClient()->SetSkill(s.first, max_skill_value);
			}
		}

		t->CastToClient()->SendLevelAppearance();

		if (RuleB(Bots, Enabled) && RuleB(Bots, BotLevelsWithOwner)) {
			Bot::LevelBotWithClient(t->CastToClient(), level, true);
		}
	}
}
