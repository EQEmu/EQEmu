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

// Theo-and-Co Phase 3 Group B — `#bot summongroup` (cleaner canonical-group
// summon). ONE idempotent command that gets your standard group up and to
// you, replacing the old spawn-each-by-name-then-summon dance:
//   1. spawn any of your owned bots that aren't up (up to the canonical
//      group size: you + 5 bots), respecting the normal spawn checks;
//   2. group every spawned bot of yours with you (creates the group if you
//      aren't grouped — reuses the stock Bot::ProcessBotGroupInvite path);
//   3. summon them all to you.
// Works whether 0 or 5 bots are already up. They arrive and then form up in
// whatever #bot formation mode is set (default Normal) — the formation
// system positions them on the next AI tick.

#include "zone/bot_command.h"

#include <list>
#include <string>

void bot_command_summon_group(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_summon_group", sep->arg[0], "summongroup")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format("Usage: {} (no arguments)", sep->arg[0]).c_str()
		);
		c->Message(Chat::White, "Spawns any of your bots that aren't up (you + 5), groups them with you, and summons them.");
		return;
	}

	if (!Bot::CheckHighEnoughLevelForBots(c)) {
		return;
	}

	if (!Bot::CheckCampSpawnConditions(c)) {
		return;
	}

	// Canonical group = the player + up to (MAX_GROUP_MEMBERS - 1) bots.
	const int canonical_cap = MAX_GROUP_MEMBERS - 1;

	// Bots of yours already in the world count toward the cap.
	int spawned_count = static_cast<int>(entity_list.GetBotListByCharacterID(c->CharacterID()).size());

	// --- 1) Spawn missing owned bots up to the canonical cap -------------
	int spawned_now = 0;
	std::list<BotsAvailableList> roster;
	if (database.botdb.LoadBotsList(c->CharacterID(), roster)) {
		for (const auto &r : roster) {
			if (spawned_count >= canonical_cap) {
				break;
			}
			if (entity_list.GetMobByBotID(r.bot_id)) {
				continue; // already spawned
			}
			if (!Bot::CheckHighEnoughLevelForBots(c, static_cast<uint8>(r.class_))) {
				continue;
			}
			if (!Bot::CheckSpawnLimit(c, static_cast<uint8>(r.class_))) {
				break; // hit the server spawn limit
			}

			auto nb = Bot::LoadBot(r.bot_id);
			if (!nb) {
				continue;
			}
			if (!nb->Spawn(c)) {
				safe_delete(nb);
				continue;
			}

			++spawned_count;
			++spawned_now;
		}
	}

	// --- 2) Group every spawned owned bot with the player ----------------
	// ProcessBotGroupInvite creates the group if the player isn't grouped
	// and is a no-op for bots already grouped, so this is idempotent.
	{
		const auto &to_group = entity_list.GetBotListByCharacterID(c->CharacterID());
		for (auto b : to_group) {
			if (!b || b->HasGroup() || b->HasRaid()) {
				continue;
			}
			Bot::ProcessBotGroupInvite(c, std::string(b->GetCleanName()));
		}
	}

	// --- 3) Summon them all to the player (mirrors #bot summon) ----------
	int summoned = 0;
	{
		const auto &to_summon = entity_list.GetBotListByCharacterID(c->CharacterID());
		for (auto b : to_summon) {
			if (!b) {
				continue;
			}

			b->WipeHateList();
			b->SetTarget(nullptr);
			b->Teleport(c->GetPosition());
			b->DoAnim(0);

			if (b->HasPet()) {
				if (b->HasControllablePet(BotAnimEmpathy::BackOff)) {
					b->GetPet()->WipeHateList();
					b->GetPet()->SetTarget(nullptr);
				}
				b->GetPet()->Teleport(c->GetPosition());
			}

			++summoned;
		}
	}

	if (summoned == 0) {
		c->Message(Chat::White, "You don't own any bots to summon. Create one with ^botcreate first.");
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Your group is ready: {} spawned now, {} grouped and summoned to you.",
			spawned_now,
			summoned
		).c_str()
	);
}
