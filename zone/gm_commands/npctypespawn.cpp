#include "zone/client.h"

void command_npctypespawn(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #npctypespawn [NPC ID] [Faction ID]");
		return;
	}

	auto npc_id = Strings::ToUnsignedInt(sep->arg[1]);
	int faction_id = 0;

	auto npc_type = content_db.LoadNPCTypesData(npc_id);
	if (npc_type) {
		auto npc = new NPC(npc_type, 0, c->GetPosition(), GravityBehavior::Water);
		if (npc) {
			if (sep->IsNumber(2)) {
				faction_id = Strings::ToInt(sep->arg[2]);
				npc->SetNPCFactionID(faction_id);
			}

			npc->AddLootTable();
			if (npc->DropsGlobalLoot()) {
				npc->CheckGlobalLootTables();
			}
			entity_list.AddNPC(npc);

			c->Message(
				Chat::White,
				fmt::format(
					"Spawned {} ({}){}.",
					npc->GetCleanName(),
					npc_id,
					(
						faction_id ?
						fmt::format(
							" on the {} Faction ({})",
							content_db.GetFactionName(faction_id),
							faction_id
						) :
						""
					)
				).c_str()
			);
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to spawn NPC ID {}.",
					npc_id
				).c_str()
			);
		}
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"NPC ID {} was not found.",
				npc_id
			).c_str()
		);
	}
}

