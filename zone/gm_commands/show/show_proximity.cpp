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

void ShowProximity(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	for (const auto& n : entity_list.GetNPCList()) {
		if (
			n.second &&
			Strings::Contains(n.second->GetName(), "Proximity")
		) {
			n.second->Depop();
		}
	}

	const auto t = c->GetTarget()->CastToNPC();

	std::vector<FindPerson_Point> v;

	FindPerson_Point p {};

	if (t->IsProximitySet()) {
		glm::vec4 position;
		position.w = t->GetHeading();
		position.x = t->GetProximityMinX();
		position.y = t->GetProximityMinY();
		position.z = t->GetZ();

		position.x = t->GetProximityMinX();
		position.y = t->GetProximityMinY();
		NPC::SpawnNodeNPC("Proximity", "", position);

		position.x = t->GetProximityMinX();
		position.y = t->GetProximityMaxY();
		NPC::SpawnNodeNPC("Proximity", "", position);

		position.x = t->GetProximityMaxX();
		position.y = t->GetProximityMinY();
		NPC::SpawnNodeNPC("Proximity", "", position);

		position.x = t->GetProximityMaxX();
		position.y = t->GetProximityMaxY();
		NPC::SpawnNodeNPC("Proximity", "", position);

		p.x = t->GetProximityMinX();
		p.y = t->GetProximityMinY();
		p.z = t->GetZ();
		v.push_back(p);

		p.x = t->GetProximityMinX();
		p.y = t->GetProximityMaxY();
		v.push_back(p);

		p.x = t->GetProximityMaxX();
		p.y = t->GetProximityMaxY();
		v.push_back(p);

		p.x = t->GetProximityMaxX();
		p.y = t->GetProximityMinY();
		v.push_back(p);

		p.x = t->GetProximityMinX();
		p.y = t->GetProximityMinY();
		v.push_back(p);
	}

	if (c->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		c->SendPathPacket(v);
	}
}
