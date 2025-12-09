#include "../../client.h"

void ShowZoneLoot(Client *c, const Seperator *sep)
{
	const uint32 search_item_id = sep->IsNumber(2) ? Strings::ToUnsignedInt(sep->arg[2]) : 0;

	std::vector<std::pair<NPC *, LootItems>> v;

	uint32 loot_count  = 0;
	uint32 loot_number = 1;

	for (auto npc_entity: entity_list.GetNPCList()) {
		auto il = npc_entity.second->GetLootItems();
		v.emplace_back(std::make_pair(npc_entity.second, il));
	}

	for (const auto &e: v) {
		NPC        *n = e.first;
		const auto &l = e.second;

		std::string npc_link;
		if (n) {
			const uint32 instance_id = zone->GetInstanceID();
			const uint32 zone_id     = zone->GetZoneID();

			const std::string &command_link = Saylink::Silent(
				fmt::format(
					"#{} {} {} {} {}",
					(instance_id != 0 ? "zoneinstance" : "zone"),
					(instance_id != 0 ? instance_id : zone_id),
					n->GetX(),
					n->GetY(),
					n->GetZ()
				),
				"Goto"
			);

			npc_link = fmt::format(
				"{} (ID {}) | {}",
				n->GetCleanName(),
				n->GetID(),
				command_link
			);
		}

		for (const auto &i: l) {
			if (!search_item_id || i->item_id == search_item_id) {
				EQ::SayLinkEngine linker;
				linker.SetLinkType(EQ::saylink::SayLinkLootItem);
				linker.SetLootData(i);

				c->Message(
					Chat::White,
					fmt::format(
						"Item {} | {}{}{}",
						loot_number,
						(
							!search_item_id ?
							fmt::format(
								"{} ({}) | ",
								linker.GenerateLink(),
								i->item_id
							) :
							""
						),
						npc_link,
						(
							!search_item_id ?
								fmt::format(
									" | {}",
									Saylink::Silent(
										fmt::format(
											"#show zone_loot {}",
											i->item_id
										),
										"Show"
									)
								) :
								""
						)
					).c_str()
				);

				loot_number++;
				loot_count++;
			}
		}
	}


	if (search_item_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} ({}) is dropping in {} place{}.",
				database.CreateItemLink(search_item_id),
				search_item_id,
				loot_count,
				loot_count != 1 ? "s" : ""
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Item{} {} dropping.",
			loot_count,
			loot_count != 1 ? "s" : "",
			loot_count != 1 ? "are" : "is"
		).c_str()
	);
}
