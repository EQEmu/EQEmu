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

void ShowAutoLogin(Client* c, const Seperator* sep)
{
	if (!RuleB(World, EnableAutoLogin)) {
		c->Message(Chat::White, "Auto login is disabled.");
		return;
	}

	Client* t = c;
	if (c->GetGM() && c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const auto& e = AccountRepository::FindOne(database, t->AccountID());

	if (!e.id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to find an account entry for {}.",
				c->GetTargetDescription(t)
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Auto login character for {} is set to '{}'.",
			c->GetTargetDescription(t),
			e.auto_login_charname
		).c_str()
	);
}
