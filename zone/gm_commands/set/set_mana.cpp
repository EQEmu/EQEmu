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

void SetMana(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set mana [Amount]");
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	int64 mana = Strings::ToBigInt(sep->arg[2]);

	if (t->IsClient()) {
		if (mana >= t->CastToClient()->CalcMaxMana()) {
			mana = t->CastToClient()->CalcMaxMana();
		}

		t->CastToClient()->SetMana(mana);
	} else {
		if (mana >= t->CalcMaxMana()) {
			mana = t->CalcMaxMana();
		}

		t->SetMana(mana);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Set {} to {} Mana.",
			c->GetTargetDescription(t),
			Strings::Commify(mana)
		).c_str()
	);
}

