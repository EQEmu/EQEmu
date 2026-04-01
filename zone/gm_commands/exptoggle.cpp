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

void command_exptoggle(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (!arguments || arguments > 1) {
		c->Message(Chat::White, "Usage: #exptoggle [Toggle] - Toggle your or your target's experience gain.");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		t = c->GetTarget()->CastToClient();
	}

	const auto is_exp_enabled = Strings::ToBool(sep->arg[1]);

	t->SetEXPEnabled(is_exp_enabled);

	c->Message(
		Chat::White,
		fmt::format(
			"Experience gain for {} is now {}abled.",
			c->GetTargetDescription(t),
			is_exp_enabled ? "en" : "dis"
		).c_str()
	);
}
