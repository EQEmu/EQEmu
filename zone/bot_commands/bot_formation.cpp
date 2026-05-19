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

// Theo-and-Co Phase 3 Group B — `#bot formation [compact | normal | spread]`.
// A group-level travel/combat SHAPE. It is honored by BOTH the combat
// positioning path (Bot::PlotBotPositionAroundTarget seeds the bot's slot)
// and the follow/travel path (Bot::TryNonCombatMovementChecks slot ring) so
// bots don't fan out while moving then snap together in combat (PHASE3_BOTS
// §1). Runtime-only by design (a transient shape, not a per-bot attribute);
// it resets to Normal on respawn.
//   compact — tight cluster on the player (~5-8u, minimal spread)
//   normal  — moderate offset formation (default)
//   spread  — wide fan (stack-punishing mechanics)

#include "zone/bot_command.h"

#include <cctype> // std::tolower
#include <string>

void bot_command_formation(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_formation", sep->arg[0], "formation")) {
		c->Message(Chat::White, "note: Set how your bots space themselves while traveling and fighting.");
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			"usage: %s [compact | normal | spread | current] "
			"[actionable: target | byname | ownergroup | ownerraid | namesgroup | "
			"healrotation | mmr | byclass | byrace | default: spawned] ([actionable_name])",
			sep->arg[0]
		);
		c->Message(Chat::White, "- compact: tight on you (corridors, traps, zone-ins, stack-for-mechanic).");
		c->Message(Chat::White, "- normal:  moderate offset formation (default).");
		c->Message(Chat::White, "- spread:  wide fan (stack-punishing AE mechanics).");
		c->Message(Chat::White, "- current: report the bots' formation mode.");
		return;
	}

	std::string verb = sep->arg[1];
	for (auto &ch : verb) {
		ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
	}

	const bool is_query = (verb == "current" || verb == "show");

	int mode = -1;
	if (!is_query) {
		if (verb == "compact" || verb == "1") {
			mode = 1;
		} else if (verb == "normal" || verb == "0") {
			mode = 0;
		} else if (verb == "spread" || verb == "2") {
			mode = 2;
		} else {
			c->Message(
				Chat::White,
				"'%s' is not a valid formation. Use: compact, normal, spread (or current).",
				sep->arg[1]
			);
			return;
		}
	}

	// arg[1] is the verb; the actionable selector starts at arg[2]
	// (mirrors bot_hold.cpp arg layout).
	const int ab_mask  = (ActionableBots::ABM_Target | ActionableBots::ABM_Type2);
	const int ab_arg   = 2;
	const int name_arg = 3;

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;
	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(
			c,
			sep->arg[ab_arg],
			sbl,
			ab_mask,
			!class_race_check ? sep->arg[name_arg] : nullptr,
			class_race_check ? atoi(sep->arg[name_arg]) : 0
		) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());
	if (sbl.empty()) {
		return;
	}

	static const char* kModeName[3] = { "Normal", "Compact", "Spread" };

	if (is_query) {
		for (auto bot_iter : sbl) {
			if (!bot_iter) {
				continue;
			}
			uint8 m = bot_iter->GetBotFormation();
			bot_iter->RaidGroupSay(
				fmt::format("Formation: {}.", kModeName[m <= 2 ? m : 0]).c_str()
			);
		}
		return;
	}

	int affected = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter) {
			continue;
		}
		bot_iter->SetBotFormation(static_cast<uint8>(mode));
		++affected;
	}

	if (sbl.size() == 1 && affected == 1) {
		sbl.front()->RaidGroupSay(
			fmt::format("Formation set to {}.", kModeName[mode]).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} of your bots set to {} formation.",
				affected,
				kModeName[mode]
			).c_str()
		);
	}
}
