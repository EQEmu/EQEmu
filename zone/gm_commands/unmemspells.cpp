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

void command_unmemspells(Client *c, const Seperator *sep)
{
	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		target = c->GetTarget()->CastToClient();
	}

	auto memmed_count = target->MemmedCount();
	if (!memmed_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} no spells to unmemorize.",
				c->GetTargetDescription(target, TargetDescriptionType::UCYou),
				c == target ? "have" : "has"
			).c_str()
		);
		return;
	}

	target->UnmemSpellAll();

	if (c != target) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} has had {} spells unmemorized.",
				c->GetTargetDescription(target),
				memmed_count
			).c_str()
		);
	}
}
