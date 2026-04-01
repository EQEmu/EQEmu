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

void ShowEmotes(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to view their emotes.");
		return;
	}

	const auto t = c->GetTarget()->CastToNPC();

	uint32       emote_count = 0;
	const uint32 emote_id    = t->GetEmoteID();

	for (auto& e : zone->npc_emote_list) {
		if (emote_id == e->emoteid) {
			c->Message(
				Chat::White,
				fmt::format(
					"Emote {} | Event: {} ({}) Type: {} ({})",
					e->emoteid,
					EQ::constants::GetEmoteEventTypeName(e->event_),
					e->event_,
					EQ::constants::GetEmoteTypeName(e->type),
					e->type
				).c_str()
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Emote {} | Text: {}",
					e->emoteid,
					e->text
				).c_str()
			);

			emote_count++;
		}

	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} has {} emote{} on Emote ID {}.",
			c->GetTargetDescription(t),
			emote_count,
			emote_count != 1 ? "s" : "",
			emote_id
		).c_str()
	);
}
