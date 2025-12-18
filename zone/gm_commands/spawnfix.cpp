#include "zone/client.h"

void command_spawnfix(Client* c, const Seperator* sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto target = c->GetTarget()->CastToNPC();
	auto spawn2 = target->respawn2;

	if (!spawn2) {
		c->Message(Chat::White, "Failed to fix spawn, the spawn must not exist in the database.");
		return;
	}

	auto client_x = c->GetX();
	auto client_y = c->GetY();
	auto client_z = target->GetFixedZ(c->GetPosition());
	auto client_heading = c->GetHeading();

	auto query = fmt::format(
		"UPDATE spawn2 SET x = {:.2f}, y = {:.2f}, z = {:.2f}, heading = {:.2f} WHERE id = {}",
		client_x,
		client_y,
		client_z,
		client_heading,
		spawn2->GetID()
	);
	auto results = content_db.QueryDatabase(query);

	if (!results.Success() || !results.RowsAffected()) {
		c->Message(Chat::White, "Failed to fix spawn.");
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Updated Spawn | NPC: {}",
			target->GetCleanName()
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Updated Spawn | NPC ID: {} Spawn2 ID: {}",
			target->GetNPCTypeID(),
			spawn2->GetID()
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Updated Spawn | Coordinates: {:.2f}, {:.2f}, {:.2f}, {:.2f}",
			client_x,
			client_y,
			client_z,
			client_heading
		).c_str()
	);

	target->Depop(false);
}

