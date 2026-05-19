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

// Theo-and-Co Phase 3 Group B — `#bot groupup` ("Group Up" button).
// Dumb-simple by design (Alex S32): group ALL of your spawned bots in the
// CURRENT ZONE into your group. No spawning, no summoning/teleporting, no
// composition logic. entity_list is per-zone, so "in your zone" is implicit.
// Grab order is just whatever order the bots are in (you control that by
// what/when you spawn). If more bots are up than a group can hold (you + 5),
// it groups what fits and tells you which bots were left out.
//
// This replaces the old `^summongroup` (spawn+group+summon) as the "Group
// Up" button — that bundled-spawn behavior was dropped (Alex S32) because it
// couldn't build a proper composition; `summongroup` stays dormant in the
// engine, just unreferenced. Smart auto-composition is a future Group C
// concern (it would use the dormant GetBotRole as its classifier).

#include "zone/bot_command.h"

#include <string>
#include <vector>

void bot_command_group_up(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_group_up", sep->arg[0], "groupup")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, fmt::format("Usage: {} (no arguments)", sep->arg[0]).c_str());
		c->Message(Chat::White, "Groups all your spawned bots in this zone with you. No spawning or summoning. If more bots are up than fit (you + 5), the extras are listed.");
		return;
	}

	if (!Bot::CheckHighEnoughLevelForBots(c)) {
		return;
	}

	if (!Bot::CheckCampSpawnConditions(c)) {
		return;
	}

	// Per-zone entity list -> this is exactly "my spawned bots in this zone".
	const auto &my_bots = entity_list.GetBotListByCharacterID(c->CharacterID());
	if (my_bots.empty()) {
		c->Message(Chat::White, "You have no spawned bots in this zone to group. Spawn some first.");
		return;
	}

	int                      grouped = 0;
	int                      already = 0;
	std::vector<std::string> left_out;

	for (auto b : my_bots) {
		if (!b) {
			continue;
		}
		if (b->HasGroup() || b->HasRaid()) {
			++already; // already in a group/raid (yours or otherwise) — leave it
			continue;
		}

		// Stock path: makes the group if you have none, else adds the bot;
		// AddBotToGroup returns false at the group cap, so an over-cap bot
		// simply stays ungrouped -> that's how we detect "left out".
		Bot::ProcessBotGroupInvite(c, std::string(b->GetCleanName()));

		if (b->HasGroup()) {
			++grouped;
		} else {
			left_out.emplace_back(b->GetCleanName());
		}
	}

	if (grouped > 0) {
		c->Message(
			Chat::White,
			fmt::format("Grouped {} bot{} with you.", grouped, grouped == 1 ? "" : "s").c_str()
		);
	} else if (left_out.empty()) {
		c->Message(Chat::White, "Your bots in this zone are already grouped.");
	}

	if (!left_out.empty()) {
		std::string names;
		for (const auto &n : left_out) {
			if (!names.empty()) {
				names += ", ";
			}
			names += n;
		}
		c->Message(
			Chat::White,
			fmt::format(
				"Group is full (you + 5) — left out: {}. Camp some bots or spawn fewer.",
				names
			).c_str()
		);
	}
}
