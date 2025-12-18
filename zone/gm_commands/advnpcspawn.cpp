#include "zone/client.h"
#include "zone/groups.h"

void command_advnpcspawn(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(
			Chat::White,
			"Usage: #advnpcspawn addentry [Spawngroup ID] [NPC ID] [Spawn Chance] - Adds a new Spawngroup Entry"
		);
		c->Message(
			Chat::White,
			"Usage: #advnpcspawn addspawn [Spawngroup ID] - Adds a new Spawngroup Entry from an existing Spawngroup"
		);
		c->Message(Chat::White, "Usage: #advnpcspawn clearbox [Spawngroup ID] - Clears the roambox of a Spawngroup");
		c->Message(Chat::White, "Usage: #advnpcspawn deletespawn - Deletes a Spawngroup");
		c->Message(
			Chat::White,
			"Usage: #advnpcspawn editbox [Spawngroup ID] [Distance] [Minimum X] [Maximum X] [Minimum Y] [Maximum Y] [Delay]  - Edit the roambox of a Spawngroup"
		);
		c->Message(
			Chat::White,
			"Usage: #advnpcspawn editrespawn [Respawn Timer] [Variance] - Edit the Respawn Timer of a Spawngroup"
		);
		c->Message(
			Chat::White,
			"Usage: #advnpcspawn makegroup [Spawn Group Name] [Spawn Limit] [Distance] [Minimum X] [Maximum X] [Minimum Y] [Maximum Y] [Delay] - Makes a new Spawngroup"
		);
		c->Message(Chat::White, "Usage: #advnpcspawn makenpc - Makes a new NPC");
		c->Message(Chat::White, "Usage: #advnpcspawn movespawn - Moves a Spawngroup to your current location");
		c->Message(Chat::White, "Usage: #advnpcspawn setversion [Version] - Sets a Spawngroup's Version");
		return;
	}

	std::string spawn_command   = Strings::ToLower(sep->arg[1]);
	bool        is_add_entry    = spawn_command.find("addentry") != std::string::npos;
	bool        is_add_spawn    = spawn_command.find("addspawn") != std::string::npos;
	bool        is_clear_box    = spawn_command.find("clearbox") != std::string::npos;
	bool        is_delete_spawn = spawn_command.find("deletespawn") != std::string::npos;
	bool        is_edit_box     = spawn_command.find("editgroup") != std::string::npos;
	bool        is_edit_respawn = spawn_command.find("editrespawn") != std::string::npos;
	bool        is_make_group   = spawn_command.find("makegroup") != std::string::npos;
	bool        is_make_npc     = spawn_command.find("makenpc") != std::string::npos;
	bool        is_move_spawn   = spawn_command.find("movespawn") != std::string::npos;
	bool        is_set_version  = spawn_command.find("setversion") != std::string::npos;
	if (
		!is_add_entry &&
		!is_add_spawn &&
		!is_clear_box &&
		!is_delete_spawn &&
		!is_edit_box &&
		!is_edit_respawn &&
		!is_make_group &&
		!is_make_npc &&
		!is_move_spawn &&
		!is_set_version
		) {
		c->Message(
			Chat::White,
			"Usage: #advnpcspawn addentry [Spawngroup ID] [NPC ID] [Spawn Chance] - Adds a new Spawngroup Entry"
		);
		c->Message(
			Chat::White,
			"Usage: #advnpcspawn addspawn [Spawngroup ID] - Adds a new Spawngroup Entry from an existing Spawngroup"
		);
		c->Message(Chat::White, "Usage: #advnpcspawn clearbox [Spawngroup ID] - Clears the roambox of a Spawngroup");
		c->Message(Chat::White, "Usage: #advnpcspawn deletespawn - Deletes a Spawngroup");
		c->Message(
			Chat::White,
			"Usage: #advnpcspawn editbox [Spawngroup ID] [Distance] [Minimum X] [Maximum X] [Minimum Y] [Maximum Y] [Delay]  - Edit the roambox of a Spawngroup"
		);
		c->Message(
			Chat::White,
			"Usage: #advnpcspawn editrespawn [Respawn Timer] [Variance] - Edit the Respawn Timer of a Spawngroup"
		);
		c->Message(
			Chat::White,
			"Usage: #advnpcspawn makegroup [Spawn Group Name] [Spawn Limit] [Distance] [Minimum X] [Maximum X] [Minimum Y] [Maximum Y] [Delay] - Makes a new Spawngroup"
		);
		c->Message(Chat::White, "Usage: #advnpcspawn makenpc - Makes a new NPC");
		c->Message(Chat::White, "Usage: #advnpcspawn movespawn - Moves a Spawngroup to your current location");
		c->Message(Chat::White, "Usage: #advnpcspawn setversion [Version] - Sets a Spawngroup's Version");
		return;
	}


	if (is_add_entry) {
		if (arguments < 4) {
			c->Message(Chat::White, "Usage: #advnpcspawn addentry [Spawngroup ID] [NPC ID] [Spawn Chance]");
			return;
		}

		auto spawngroup_id = Strings::ToUnsignedInt(sep->arg[2]);
		auto npc_id = Strings::ToUnsignedInt(sep->arg[3]);
		auto spawn_chance = Strings::ToUnsignedInt(sep->arg[4]);

		auto query = fmt::format(
			SQL(
				INSERT INTO spawnentry (spawngroupID, npcID, chance)
				VALUES ({}, {}, {})
			),
			spawngroup_id,
			npc_id,
			spawn_chance
		);
		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::White, "Failed to add entry to Spawngroup.");
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"{} ({}) added to Spawngroup {}, its spawn chance is {}%%.",
				database.GetCleanNPCNameByID(npc_id),
				npc_id,
				spawngroup_id,
				spawn_chance
			).c_str()
		);
		return;
	} else if (is_add_spawn) {
		if (
			content_db.NPCSpawnDB(
				NPCSpawnTypes::AddSpawnFromSpawngroup,
				zone->GetShortName(),
				zone->GetInstanceVersion(),
				c,
				0,
				Strings::ToUnsignedInt(sep->arg[2])
			)
		) {
			c->Message(
				Chat::White,
				fmt::format(
					"Spawn Added | Added spawn from Spawngroup ID {}.",
					Strings::ToUnsignedInt(sep->arg[2])
				).c_str()
			);
		}
		return;
	} else if (is_clear_box) {
		if (arguments != 2 || !sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #advnpcspawn clearbox [Spawngroup ID]");
			return;
		}

		auto query = fmt::format(
			"UPDATE spawngroup SET dist = 0, min_x = 0, max_x = 0, min_y = 0, max_y = 0, delay = 0 WHERE id = {}",
			Strings::ToUnsignedInt(sep->arg[2])
		);
		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::White, "Failed to clear Spawngroup box.");
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Spawngroup {} Roambox Cleared | Delay: 0 Distance: 0.00",
				Strings::ToUnsignedInt(sep->arg[2])
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Spawngroup {} Roambox Cleared | Minimum X: 0.00 Maximum X: 0.00",
				Strings::ToUnsignedInt(sep->arg[2])
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Spawngroup {} Roambox Cleared | Minimum Y: 0.00 Maximum Y: 0.00",
				Strings::ToUnsignedInt(sep->arg[2])
			).c_str()
		);
		return;
	} else if (is_delete_spawn) {
		if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
			c->Message(Chat::White, "You must target an NPC to use this command.");
			return;
		}

		auto target = c->GetTarget()->CastToNPC();
		auto spawn2 = target->respawn2;
		if (!spawn2) {
			c->Message(Chat::White, "Failed to delete spawn because NPC has no Spawn2.");
			return;
		}

		auto spawn2_id = spawn2->GetID();
		auto query = fmt::format(
			"DELETE FROM spawn2 WHERE id = {}",
			spawn2_id
		);
		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::White, "Failed to delete spawn.");
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Spawn2 {} Deleted | Name: {}",
				spawn2_id,
				c->GetTargetDescription(target)
			).c_str()
		);
		target->Depop(false);
		return;
	} else if (is_edit_box) {
		if (
			arguments != 8 ||
			!sep->IsNumber(3) ||
			!sep->IsNumber(4) ||
			!sep->IsNumber(5) ||
			!sep->IsNumber(6) ||
			!sep->IsNumber(7) ||
			!sep->IsNumber(8)
		) {
			c->Message(
				Chat::White,
				"Usage: #advnpcspawn editbox [Spawngroup ID] [Distance] [Minimum X] [Maximum X] [Minimum Y] [Maximum Y] [Delay]"
			);
			return;
		}
		auto spawngroup_id = Strings::ToUnsignedInt(sep->arg[2]);
		auto distance = Strings::ToFloat(sep->arg[3]);
		auto minimum_x = Strings::ToFloat(sep->arg[4]);
		auto maximum_x = Strings::ToFloat(sep->arg[5]);
		auto minimum_y = Strings::ToFloat(sep->arg[6]);
		auto maximum_y = Strings::ToFloat(sep->arg[7]);
		auto delay = Strings::ToInt(sep->arg[8]);

		auto query = fmt::format(
			"UPDATE spawngroup SET dist = {:.2f}, min_x = {:.2f}, max_x = {:.2f}, max_y = {:.2f}, min_y = {:.2f}, delay = {} WHERE id = {}",
			distance,
			minimum_x,
			maximum_x,
			minimum_y,
			maximum_y,
			delay,
			spawngroup_id
		);
		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::White, "Failed to edit Spawngroup box.");
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Spawngroup {} Roambox Edited | Delay: {} Distance: {:.2f}",
				spawngroup_id,
				delay,
				distance
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Spawngroup {} Roambox Edited | Minimum X: {:.2f} Maximum X: {:.2f}",
				spawngroup_id,
				minimum_x,
				maximum_x
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Spawngroup {} Roambox Edited | Minimum Y: {:.2f} Maximum Y: {:.2f}",
				spawngroup_id,
				minimum_y,
				maximum_y
			).c_str()
		);
		return;
	} else if (is_edit_respawn) {
		if (arguments < 2 || !sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #advnpcspawn editrespawn [Respawn Timer] [Variance]");
			return;
		}

		if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
			c->Message(Chat::White, "You must target an NPC to use this command.");
			return;
		}

		auto target = c->GetTarget()->CastToNPC();
		auto spawn2 = target->respawn2;
		if (!spawn2) {
			c->Message(Chat::White, "Failed to edit respawn because NPC has no Spawn2.");
			return;
		}

		auto spawn2_id = spawn2->GetID();
		auto respawn_timer = Strings::ToUnsignedInt(sep->arg[2]);
		auto variance = sep->IsNumber(3) ? Strings::ToUnsignedInt(sep->arg[3]) : spawn2->GetVariance();

		auto query = fmt::format(
			"UPDATE spawn2 SET respawntime = {}, variance = {} WHERE id = {}",
			respawn_timer,
			variance,
			spawn2_id
		);
		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::White, "Failed to edit respawn.");
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Spawn2 {} Respawn Modified | Name: {} Respawn Timer: {} Variance: {}",
				spawn2_id,
				c->GetTargetDescription(target),
				respawn_timer,
				variance
			).c_str()
		);

		spawn2->SetRespawnTimer(respawn_timer);
		spawn2->SetVariance(variance);
		return;
	} else if (is_make_group) {
		if (arguments < 2) {
			c->Message(
				Chat::White,
				"Usage: #advncspawn makegroup [Spawn Group Name] [Spawn Limit] [Distance] [Minimum X] [Maximum X] [Minimum Y] [Maximum Y] [Delay]"
			);
			return;
		}

		std::string spawngroup_name = sep->arg[2];
		auto spawn_limit = sep->IsNumber(3) ? Strings::ToInt(sep->arg[3]) : 0;
		auto distance = sep->IsNumber(4) ? Strings::ToFloat(sep->arg[4]) : 0.0f;
		auto minimum_x = sep->IsNumber(5) ? Strings::ToFloat(sep->arg[5]) : 0.0f;
		auto maximum_x = sep->IsNumber(6) ? Strings::ToFloat(sep->arg[6]) : 0.0f;
		auto minimum_y = sep->IsNumber(7) ? Strings::ToFloat(sep->arg[7]) : 0.0f;
		auto maximum_y = sep->IsNumber(8) ? Strings::ToFloat(sep->arg[8]) : 0.0f;
		auto delay = sep->IsNumber(9) ? Strings::ToInt(sep->arg[9]) : 0;

		auto query = fmt::format(
			"INSERT INTO spawngroup"
			"(name, spawn_limit, dist, min_x, max_x, min_y, max_y, delay)"
			"VALUES ('{}', {}, {:.2f}, {:.2f}, {:.2f}, {:.2f}, {:.2f}, {})",
			spawngroup_name,
			spawn_limit,
			distance,
			minimum_x,
			maximum_x,
			minimum_y,
			maximum_y,
			delay
		);
		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::White, "Failed to make Spawngroup.");
			return;
		}

		auto spawngroup_id = results.LastInsertedID();
		c->Message(
			Chat::White,
			fmt::format(
				"Spawngroup {} Created | Name: {} Spawn Limit: {}",
				spawngroup_id,
				spawngroup_name,
				spawn_limit
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"Spawngroup {} Created | Delay: {} Distance: {:.2f}",
				spawngroup_id,
				delay,
				distance
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"Spawngroup {} Created | Minimum X: {:.2f} Maximum X: {:.2f}",
				spawngroup_id,
				minimum_x,
				maximum_x
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"Spawngroup {} Created | Minimum Y: {:.2f} Maximum Y: {:.2f}",
				spawngroup_id,
				minimum_y,
				maximum_y
			).c_str()
		);
		return;
	} else if (is_make_npc) {
		if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
			c->Message(Chat::White, "You must target an NPC to use this command.");
			return;
		}

		if (
			content_db.NPCSpawnDB(
				NPCSpawnTypes::CreateNewNPC,
				zone->GetShortName(),
				zone->GetInstanceVersion(),
				c,
				c->GetTarget()->CastToNPC()
			)
		) {
			c->Message(Chat::White, "Created a new NPC.");
		}

		return;
	} else if (is_move_spawn) {
		if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
			c->Message(Chat::White, "You must target an NPC to use this command.");
			return;
		}

		auto target = c->GetTarget()->CastToNPC();
		auto spawn2 = target->respawn2;
		if (!spawn2) {
			c->Message(Chat::White, "Failed to move spawn because NPC has no Spawn2.");
			return;
		}

		auto client_position = c->GetPosition();
		auto spawn2_id = spawn2->GetID();

		auto query = fmt::format(
			"UPDATE spawn2 SET x = {:.2f}, y = {:.2f}, z = {:.2f}, heading = {:.2f} WHERE id = {}",
			client_position.x,
			client_position.y,
			client_position.z,
			client_position.w,
			spawn2_id
		);
		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::White, "Failed to move spawn.");
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Spawn2 {} Moved | Name: {}",
				spawn2_id,
				c->GetTargetDescription(target)
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Spawn2 {} Moved | XYZ: {}, {}, {} Heading: {}",
				spawn2_id,
				client_position.x,
				client_position.y,
				client_position.z,
				client_position.w
			).c_str()
		);

		target->GMMove(
			client_position.x,
			client_position.y,
			client_position.z,
			client_position.w
		);
		return;
	} else if (is_set_version) {
		if (arguments != 2 || !sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #advnpcspawn setversion [Version]");
			return;
		}

		if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
			c->Message(Chat::White, "You must target an NPC to use this command.");
			return;
		}

		auto target = c->GetTarget()->CastToNPC();
		auto version = Strings::ToUnsignedInt(sep->arg[2]);

		auto query = fmt::format(
			"UPDATE spawn2 SET version = {} WHERE spawngroupID = {}",
			version,
			target->GetSpawnGroupId()
		);
		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::White, "Failed to set version.");
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Spawngroup {} Version Modified | Name: {} Version: {}",
				target->GetSpawnGroupId(),
				c->GetTargetDescription(target),
				version
			).c_str()
		);
		target->Depop(false);
		return;
	}
}

