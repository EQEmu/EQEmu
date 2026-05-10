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

void SetUnassignClass(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set unassign_class [Class ID] (removes class from multiclass mask, cannot remove base class)");
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

	if (!t->HasAssignedClass(class_id)) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"{} does not have {} ({}) assigned.",
				c->GetTargetDescription(t),
				GetClassIDName(class_id),
				class_id
			).c_str()
		);
		return;
	}

	if (class_id == t->GetClass()) {
		c->Message(
			Chat::Red,
			fmt::format(
				"Cannot unassign base class {} ({}). Use #set class_permanent to change the base class first.",
				GetClassIDName(class_id),
				class_id
			).c_str()
		);
		return;
	}

	uint32 new_mask = t->GetClassMask() & ~GetPlayerClassBit(class_id);

	if (!t->SetClassMask(new_mask)) {
		c->Message(Chat::Red, "Failed to unassign class.");
		return;
	}

	t->Save();

	LogInfo(
		"Class {} unassigned by {} for {}.",
		GetClassIDName(class_id),
		c->GetCleanName(),
		c->GetTargetDescription(t)
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Unassigned {} ({}) from {}. New mask: {}.",
			GetClassIDName(class_id),
			class_id,
			c->GetTargetDescription(t),
			t->GetClassMask()
		).c_str()
	);
}
