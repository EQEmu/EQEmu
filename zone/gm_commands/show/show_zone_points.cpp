#include "../../client.h"

void ShowZonePoints(Client *c, const Seperator *sep)
{
	for (const auto& m : entity_list.GetMobList()) {
		Mob* mob = m.second;
		if (mob->IsNPC() && mob->GetRace() == Race::Node) {
			mob->Depop();
		}
	}

	uint32 found_count = 0;

	c->Message(Chat::White, "Listing zone points...");

	c->SendChatLineBreak();

	for (auto &p : zone->virtual_zone_point_list) {
		const std::string& zone_long_name = ZoneLongName(p.target_zone_id);

		const std::string& saylink = fmt::format(
			"#goto {:.0f} {:.0f} {:.0f}",
			p.x,
			p.y,
			p.z
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Virtual Zone Point [{}] x [{}] y [{}] z [{}] h [{}] width [{}] height [{}] | To [{}] ({}) x [{}] y [{}] z [{}] h [{}]",
				Saylink::Silent(saylink, "Goto").c_str(),
				p.x,
				p.y,
				p.z,
				p.heading,
				p.width,
				p.height,
				zone_long_name.c_str(),
				p.target_zone_id,
				p.target_x,
				p.target_y,
				p.target_z,
				p.target_heading
			).c_str()
		);

		const std::string& node_name = fmt::format("ZonePoint To [{}]", zone_long_name);

		float half_width = ((float) p.width / 2);

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				p.x + half_width,
				p.y + half_width,
				p.z,
				p.heading
			)
		);

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				p.x + half_width,
				p.y - half_width,
				p.z,
				p.heading
			)
		);

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				p.x - half_width,
				p.y - half_width,
				p.z,
				p.heading
			)
		);

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				p.x - half_width,
				p.y + half_width,
				p.z,
				p.heading
			)
		);

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				p.x + half_width,
				p.y + half_width,
				p.z + static_cast<float>(p.height),
				p.heading
			)
		);

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				p.x + half_width,
				p.y - half_width,
				p.z + static_cast<float>(p.height),
				p.heading
			)
		);

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				p.x - half_width,
				p.y - half_width,
				p.z + static_cast<float>(p.height),
				p.heading
			)
		);

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				p.x - half_width,
				p.y + half_width,
				p.z + static_cast<float>(p.height),
				p.heading
			)
		);

		found_count++;
	}

	LinkedListIterator<ZonePoint *> iterator(zone->zone_point_list);
	iterator.Reset();
	while (iterator.MoreElements()) {
		const auto &p = iterator.GetData();

		const std::string& zone_long_name = ZoneLongName(p->target_zone_id);
		const std::string& node_name      = fmt::format("ZonePoint To [{}]", zone_long_name);

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				p->x,
				p->y,
				p->z,
				p->heading
			)
		);

		const std::string& saylink = fmt::format(
			"#goto {:.0f} {:.0f} {:.0f}",
			p->x,
			p->y,
			p->z
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Client Side Zone Point [{}] x [{}] y [{}] z [{}] h [{}] number [{}] | To [{}] ({}) x [{}] y [{}] z [{}] h [{}]",
				Saylink::Silent(saylink, "Goto"),
				p->x,
				p->y,
				p->z,
				p->heading,
				p->number,
				zone_long_name,
				p->target_zone_id,
				p->target_x,
				p->target_y,
				p->target_z,
				p->target_heading
			).c_str()
		);

		iterator.Advance();

		found_count++;
	}

	if (!found_count) {
		c->Message(Chat::White, "There were no zone points found.");
	}

	c->SendChatLineBreak();
}
