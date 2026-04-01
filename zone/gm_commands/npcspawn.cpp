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

void command_npcspawn(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Command Syntax: #npcspawn [Add|Create|Delete|Remove|Update|Clone|Help]");
		return;
	}

	if (!strcasecmp(sep->arg[1], "help")) {
		c->Message(Chat::White, "Command Syntax: #npcspawn [Add|Create|Delete|Remove|Update|Clone|Help] [optional 3rd parameter]");
		c->Message(Chat::White, "Usage: #npcspawn add [respawntime] - Using the same targeted NPC ID, creates new spawn2 and spawngroup entries");
		c->Message(Chat::White, "Usage: #npcspawn clone [respawntime] - Copies targeted NPC and spawngroup, creating only a spawn2 entry at the current client location");
		c->Message(Chat::White, "Usage: #npcspawn create [respawntime] - Creates new NPC type copying the data from the targeted NPC, with new spawn2 and spawngroup entries");
		c->Message(Chat::White, "Usage: #npcspawn delete - Deletes the spawn2, spawngroup, spawnentry and npc_types rows for targeted NPC");
		c->Message(Chat::White, "Usage: #npcspawn remove [remove_spawngroups] - Deletes the spawn2 row for targeted NPC, also delete spawngroup and spawnentry rows if remove_spawngroups is > 0");
		c->Message(Chat::White, "Usage: #npcspawn update - Updates NPC appearance in database");
		return;
	}

	auto target = c->GetTarget()->CastToNPC();
	uint32 extra = 0;
	bool is_add = !strcasecmp(sep->arg[1], "add");
	bool is_clone = !strcasecmp(sep->arg[1], "clone");
	bool is_create = !strcasecmp(sep->arg[1], "create");
	bool is_delete = !strcasecmp(sep->arg[1], "delete");
	bool is_remove = !strcasecmp(sep->arg[1], "remove");
	bool is_update = !strcasecmp(sep->arg[1], "update");
	if (
		!is_add &&
		!is_clone &&
		!is_create &&
		!is_delete &&
		!is_remove &&
		!is_update
	) {
		c->Message(Chat::White, "Command Syntax: #npcspawn [Add|Create|Delete|Remove|Update|Clone|Help]");
		return;
	}

	if (is_add || is_create || is_clone) {
		extra = sep->IsNumber(2) ? Strings::ToInt(sep->arg[2]) : 1200; // Extra param is only used for respawn time in Add/Create/Clone, default to 1200 if not set

		content_db.NPCSpawnDB(
			is_add ? NPCSpawnTypes::AddNewSpawngroup : (is_create ? NPCSpawnTypes::CreateNewSpawn : NPCSpawnTypes::AddSpawnFromSpawngroup),
			zone->GetShortName(),
			zone->GetInstanceVersion(),
			c,
			target,
			extra
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Spawn {} | Name: {}",
				is_add ?
				"Added" :
				(
					is_create ?
					"Created" :
					"Cloned"
				),
				c->GetTargetDescription(target)
			).c_str()
		);
	}
	else if (is_delete || is_remove || is_update) {
		uint8 spawn_update_type = (
			is_delete ?
			NPCSpawnTypes::DeleteSpawn :
			(
				is_remove ?
				NPCSpawnTypes::RemoveSpawn :
				NPCSpawnTypes::UpdateAppearance
			)
		);

		std::string spawn_message = (
			is_delete ?
			"Deleted" :
			(
				is_remove ?
				"Removed" :
				"Updated"
			)
		);

		extra = sep->IsNumber(2) ? Strings::ToInt(sep->arg[2]) : 0; // Extra param is used in Remove as a flag to optionally remove spawngroup/spawnentry if 1 (always remove spawn2 entry)

		content_db.NPCSpawnDB(
			spawn_update_type,
			zone->GetShortName(),
			zone->GetInstanceVersion(),
			c,
			target,
			extra
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Spawn {} | Name: {}",
				spawn_message,
				c->GetTargetDescription(target)
			).c_str()
		);
	}
}

