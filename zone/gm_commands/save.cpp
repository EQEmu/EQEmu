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
#include "zone/corpse.h"

void command_save(Client *c, const Seperator *sep)
{
	if (
		!c->GetTarget() ||
		(
			c->GetTarget() &&
			!c->GetTarget()->IsClient() &&
			!c->GetTarget()->IsPlayerCorpse()
		)
	) {
		c->Message(Chat::White, "You must target a player or player corpse to use this command.");
		return;
	}

	auto target = c->GetTarget();

	if (target->IsClient()) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} been {} saved.",
				c->GetTargetDescription(target, TargetDescriptionType::UCYou),
				c == target ? "have" : "has",
				target->CastToClient()->Save(2) ? "successfully" : "failed to be"
			).c_str()
		);
	} else if (target->IsPlayerCorpse()) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} has been {} saved.",
				c->GetTargetDescription(target),
				target->CastToMob()->Save() ? "successfully" : "failed to be"
			).c_str()
		);
	}
}

