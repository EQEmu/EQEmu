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

void SetClassPermanent(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set class_permanent [Class ID]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint32 class_id = Strings::ToUnsignedInt(sep->arg[2]);

	LogInfo("Class changed by {} for {} to {} ({}).",
		c->GetCleanName(),
		c->GetTargetDescription(t),
		GetClassIDName(class_id),
		class_id
	);

	t->SetBaseClass(class_id);
	t->Save();
	t->Kick("Class was changed.");

	if (c != t) {
		c->Message(
			Chat::White,
			fmt::format(
				"Class changed for {} to {} ({}).",
				c->GetTargetDescription(t),
				GetClassIDName(class_id),
				class_id
			).c_str()
		);
	}
}
