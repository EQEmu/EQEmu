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

void ShowNPCType(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #show npc_type [NPC ID]");
		return;
	}

	const uint32 npc_id = Strings::ToUnsignedInt(sep->arg[2]);
	const auto   d      = content_db.LoadNPCTypesData(npc_id);

	if (!d) {
		c->Message(
			Chat::White,
			fmt::format(
				"NPC ID {} was not found.",
				npc_id
			).c_str()
		);

		return;
	}

	auto npc = new NPC(
		d,
		nullptr,
		c->GetPosition(),
		GravityBehavior::Water
	);

	npc->ShowStats(c);

	safe_delete(npc);
}
