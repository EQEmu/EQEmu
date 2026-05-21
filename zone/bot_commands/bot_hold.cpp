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
#include "zone/bot_command.h"

void bot_command_hold(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_hold", sep->arg[0], "hold")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {

		c->Message(Chat::White, "usage: %s ([option: clear]) [actionable: byname | ownergroup | ownerraid | namesgroup | healrotation | mmr | byclass | byrace | default: spawned] ([actionable_name])", sep->arg[0]);
		return;
	}
	const int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_Type2);

	bool clear = false;
	int ab_arg = 1;
	int name_arg = 2;

	std::string clear_arg = sep->arg[1];
	if (!clear_arg.compare("clear")) {

		clear = true;
		++ab_arg;
		++name_arg;
	}

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;
	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, !class_race_check ? sep->arg[name_arg] : nullptr, class_race_check ? atoi(sep->arg[name_arg]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	// S39 fix #7: track how many bots actually transitioned state. The
	// new Attack button (client-pack v1.4.15) issues `^hold clear spawned`
	// before `^attack spawned` on every click -- without this gate, every
	// Attack click would spam "N of your bots are no longer holding their
	// attacks" even when no bots were held.
	int changed_count = 0;
	Bot* first_changed = nullptr;
	for (auto bot_iter : sbl) {

		if (clear) {
			if (bot_iter->GetHoldFlag()) {
				bot_iter->SetHoldFlag(false);
				++changed_count;
				if (!first_changed) {
					first_changed = bot_iter;
				}
			}
		}
		else {
			bot_iter->SetHoldMode();
			++changed_count;
			if (!first_changed) {
				first_changed = bot_iter;
			}
		}
	}

	if (changed_count == 0) {
		// Silent no-op (typically: ^hold clear fired on already-unheld bots).
		return;
	}

	if (changed_count == 1 && first_changed) {
		first_changed->RaidGroupSay(
			fmt::format(
				"{}olding my attacks.",
				clear ? "No longer h" : "H"
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} of your bots are {}holding their attacks.",
				changed_count,
				clear ? "no longer " : ""
			).c_str()
		);
	}
}
