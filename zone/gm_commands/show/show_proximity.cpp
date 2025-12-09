#include "../../client.h"

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
