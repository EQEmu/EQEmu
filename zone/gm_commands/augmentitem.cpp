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
#include "zone/object.h"

void command_augmentitem(Client *c, const Seperator *sep)
{
	if (!c) {
		return;
	}

	auto in_augment = new AugmentItem_Struct[sizeof(AugmentItem_Struct)];
	in_augment->container_slot = 1000; // <watch>
	in_augment->augment_slot   = -1;
	if (c->GetTradeskillObject() != nullptr) {
		Object::HandleAugmentation(c, in_augment, c->GetTradeskillObject());
	}
	safe_delete_array(in_augment);
}

