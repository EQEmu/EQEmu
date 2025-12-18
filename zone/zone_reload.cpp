/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "zone_reload.h"

#include "zone/quest_parser_collection.h"

void ZoneReload::HotReloadQuests()
{
	BenchTimer timer;

	entity_list.ClearAreas();

	parse->ReloadQuests(RuleB(HotReload, QuestsResetTimersWithReload));

	if (RuleB(HotReload, QuestsRepopWithReload)) {
		zone->Repop();
	}

	zone->SetQuestHotReloadQueued(false);

	LogHotReload(
		"Reloading [{}] repop [{}] reset_timers [{}] repop_when_not_in_combat [{}] Time [{:.4f}]",
		zone->GetShortName(),
		(RuleB(HotReload, QuestsRepopWithReload) ? "true" : "false"),
		(RuleB(HotReload, QuestsResetTimersWithReload) ? "true" : "false"),
		(RuleB(HotReload, QuestsRepopWhenPlayersNotInCombat) ? "true" : "false"),
		timer.elapsed()
	);
}
