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

// Theo-and-Co Phase 3 Group B — `#bot role`.
// A bot's role (Tank/Healer/DPS/CC/Support) is class-derived by default
// (bot_stat_model.h GetBotRole) and drives BOTH its stat distribution
// (Group A role multipliers) and, going forward, its AI/positioning.
// This command lets a player OVERRIDE that per bot; the override is
// persisted (bot_roles table) so it survives respawn, and applied LIVE
// (CalcBotStats -> CalcBonuses re-derives the stat model immediately, no
// respawn needed). Designed to be the future replacement for `#bot stance`
// and the group-queryable role surface Group C reactive AI builds on
// (Bot::GetEffectiveBotRole()).

#include "zone/bot_command.h"
#include "zone/bot_stat_model.h" // BotRole, ParseBotRole, GetBotRoleName, IsValidBotRole

void bot_command_role(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_role", sep->arg[0], "role")) {
		c->Message(Chat::White, "note: Set a bot's role. Role drives its stat focus and combat behavior.");
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			"usage: %s [tank | healer | dps | cc | support | current | reset] "
			"[actionable: target | byname | ownergroup | ownerraid | namesgroup | "
			"healrotation | mmr | byclass | byrace | default: spawned] ([actionable_name])",
			sep->arg[0]
		);
		c->Message(Chat::White, "- tank/healer/dps/cc/support: pin the bot to that role (persists, re-derives stats live).");
		c->Message(Chat::White, "- current: report the bot's effective role.");
		c->Message(Chat::White, "- reset: clear the override (back to the class-derived default).");
		return;
	}

	std::string verb = sep->arg[1];
	for (auto &ch : verb) {
		ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
	}

	const bool is_query = (verb == "current" || verb == "show" || verb == "list");
	const bool is_reset = (verb == "reset" || verb == "default" || verb == "clear");

	int role_id = -1;
	if (!is_query && !is_reset) {
		role_id = ParseBotRole(verb);
		if (!IsValidBotRole(role_id)) {
			c->Message(
				Chat::White,
				"'%s' is not a valid role. Use: tank, healer, dps, cc, support (or current | reset).",
				sep->arg[1]
			);
			return;
		}
	}

	// arg[1] is the verb/role; the actionable selector starts at arg[2]
	// (mirrors the bot_hold.cpp arg layout).
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

	int affected = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter) {
			continue;
		}

		if (is_query) {
			BotRole eff = static_cast<BotRole>(bot_iter->GetEffectiveBotRole());
			bool overridden = (bot_iter->GetBotRoleOverride() >= 0);
			bot_iter->RaidGroupSay(
				fmt::format(
					"My role is {} ({}).",
					GetBotRoleName(eff),
					overridden ? "pinned" : "class default"
				).c_str()
			);
			continue;
		}

		if (is_reset) {
			bot_iter->SetDefaultBotRole();
		} else {
			bot_iter->SetBotRoleOverride(static_cast<int8>(role_id));
		}

		// Persist (bot_roles) so it survives respawn, then re-derive the
		// stat model LIVE — CalcBotStats -> CalcBonuses applies the new
		// role's multipliers immediately, no respawn.
		database.botdb.SaveBotRole(bot_iter);
		bot_iter->CalcBotStats(false);
		++affected;
	}

	if (is_query) {
		return;
	}

	if (sbl.size() == 1 && affected == 1) {
		Bot* b = sbl.front();
		sbl.front()->RaidGroupSay(
			fmt::format(
				"My role is now {}.",
				GetBotRoleName(static_cast<BotRole>(b->GetEffectiveBotRole()))
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} of your bots {} {}.",
				affected,
				affected == 1 ? "is now" : "are now",
				is_reset
					? "back to their class-default role"
					: fmt::format("set to {}", GetBotRoleName(static_cast<BotRole>(role_id)))
			).c_str()
		);
	}
}
