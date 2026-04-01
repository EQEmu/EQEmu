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

void FindEmote(Client *c, const Seperator *sep)
{
	uint32 found_count = 0;

	if (sep->IsNumber(2)) {
		auto emote_id = Strings::ToUnsignedInt(sep->arg[2]);

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

				found_count++;
			}

			if (found_count == 50) {
				break;
			}

		}

		if (found_count == 50) {
			c->Message(
				Chat::White,
				fmt::format(
					"50 Emotes shown matching ID '{}', max reached.",
					emote_id
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"{} Emote{} found matching ID '{}'.",
				found_count,
				found_count != 1 ? "s" : "",
				emote_id
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = sep->argplus[2];

	for (auto& e : zone->npc_emote_list) {

		const std::string& current_text = Strings::ToLower(e->text);

		if (Strings::Contains(current_text, Strings::ToLower(search_criteria))) {
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

			found_count++;
		}

		if (found_count == 50) {
			break;
		}

	}

	if (found_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 Emotes shown matching '{}', max reached.",
				search_criteria
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Emote{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			search_criteria
		).c_str()
	);
}
