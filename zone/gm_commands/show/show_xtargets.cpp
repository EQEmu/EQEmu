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
#include "common/data_verification.h"
#include "zone/client.h"

void ShowXTargets(Client *c, const Seperator *sep)
{
	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		t->ShowXTargets(c);
		return;
	}

	const auto new_max = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));

	if (!EQ::ValueWithin(new_max, 5, XTARGET_HARDCAP)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Number of XTargets must be between 5 and {}.",
				XTARGET_HARDCAP
			).c_str()
		);
		return;
	}

	t->SetMaxXTargets(new_max);

	c->Message(
		Chat::White,
		fmt::format(
			"Max number of XTargets set to {} for {}.",
			new_max,
			c->GetTargetDescription(t)
		).c_str()
	);
}
