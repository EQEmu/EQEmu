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

void SetClassMask(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set class_mask [Mask] (bitmask of assigned classes, max 3)");
		c->Message(Chat::White, "Example: #set class_mask 5 (Warrior + Paladin)");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	if (c != t && c->Admin() < RuleI(GM, MinStatusToLevelTarget)) {
		c->Message(Chat::White, "Your status is not high enough to modify another character's class mask.");
		return;
	}

	const uint32 new_mask = Strings::ToUnsignedInt(sep->arg[2]);

	if (!t->SetClassMask(new_mask)) {
		c->Message(
			Chat::Red,
			fmt::format(
				"Failed to set class mask to {}. Mask must include base class ({}), contain only valid player class bits, and have at most 3 classes.",
				new_mask,
				GetClassIDName(t->GetClass())
			).c_str()
		);
		return;
	}

	t->Save();

	LogInfo(
		"Class mask changed by {} for {} to {}.",
		c->GetCleanName(),
		c->GetTargetDescription(t),
		new_mask
	);

	std::string class_list;
	auto assigned = t->GetAssignedClasses();
	for (size_t i = 0; i < assigned.size(); ++i) {
		if (i > 0) class_list += ", ";
		class_list += fmt::format("{} ({})", GetClassIDName(assigned[i]), assigned[i]);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Class mask for {} set to {} [{}].",
			c->GetTargetDescription(t),
			new_mask,
			class_list
		).c_str()
	);
}
