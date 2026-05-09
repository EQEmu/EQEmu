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

void SetAssignClass(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set assign_class [Class ID] (1-16, adds class to multiclass mask)");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	if (c != t && c->Admin() < RuleI(GM, MinStatusToLevelTarget)) {
		c->Message(Chat::White, "Your status is not high enough to modify another character's classes.");
		return;
	}

	const uint8 class_id = Strings::ToUnsignedInt(sep->arg[2]);

	if (!IsPlayerClass(class_id)) {
		c->Message(Chat::Red, fmt::format("Class ID {} is not a valid player class (1-16).", class_id).c_str());
		return;
	}

	if (t->HasAssignedClass(class_id)) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"{} already has {} ({}) assigned.",
				c->GetTargetDescription(t),
				GetClassIDName(class_id),
				class_id
			).c_str()
		);
		return;
	}

	uint32 new_mask = t->GetClassMask() | GetPlayerClassBit(class_id);

	if (!t->SetClassMask(new_mask)) {
		c->Message(
			Chat::Red,
			fmt::format(
				"Failed to assign {}. Maximum of 3 classes allowed.",
				GetClassIDName(class_id)
			).c_str()
		);
		return;
	}

	t->Save();

	LogInfo(
		"Class {} assigned by {} for {}.",
		GetClassIDName(class_id),
		c->GetCleanName(),
		c->GetTargetDescription(t)
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Assigned {} ({}) to {}. New mask: {}.",
			GetClassIDName(class_id),
			class_id,
			c->GetTargetDescription(t),
			t->GetClassMask()
		).c_str()
	);
}
