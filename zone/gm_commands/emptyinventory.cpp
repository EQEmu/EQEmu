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

void command_emptyinventory(Client *c, const Seperator *sep)
{
	Client* t = c;
	if (c->GetGM() && c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	EQ::ItemInstance *item = nullptr;
	uint32 removed_count = 0;

	for (const int16& slot_id : t->GetInventorySlots()) {
		item = t->GetInv().GetItem(slot_id);

		if (item) {
			uint32 stack_size = std::max(static_cast<uint32>(item->GetCharges()), static_cast<uint32>(1));
			removed_count += stack_size;
			t->DeleteItemInInventory(slot_id, 0, true);
		}
	}

	if (!removed_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} no items to delete.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "have" : "has"
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Inventory cleared for {}, {} item{} deleted.",
			c->GetTargetDescription(t),
			removed_count,
			removed_count != 1 ? "s" : ""
		).c_str()
	);
}
