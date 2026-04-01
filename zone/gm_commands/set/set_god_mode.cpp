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
#include "common/repositories/account_repository.h"
#include "zone/client.h"

void SetGodMode(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set god_mode [on|off]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const bool   god_mode   = Strings::ToBool(sep->arg[2]);
	const uint32 account_id = c->AccountID();

	auto a = AccountRepository::FindOne(database, c->AccountID());
	if (a.id) {
		a.flymode      = god_mode ? 1 : 0;
		a.gmspeed      = god_mode ? 1 : 0;
		a.invulnerable = god_mode ? 1 : 0;
		a.hideme       = god_mode ? 1 : 0;
	}

	c->SetInvul(god_mode);
	c->SendAppearancePacket(AppearanceType::FlyMode, god_mode);
	c->SetHideMe(god_mode);

	c->Message(
		Chat::White,
		fmt::format(
			"Turning God Mode {} for {}, zone for GM Speed to take effect.",
			god_mode ? "on" : "off",
			c->GetTargetDescription(t)
		).c_str()
	);

	AccountRepository::UpdateOne(database, a);
}
