#include "zone/client.h"
#include "zone/worldserver.h"

extern WorldServer worldserver;

void command_summon(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;
	if (!arguments && !c->GetTarget()) {
		c->Message(Chat::White, "Usage: #summon - Summon your target, if you have one, to your position");
		c->Message(Chat::White, "Usage: #summon [Character Name] - Summon a character by name to your position");
		return;
	}

	Mob *t = c;

	if (arguments == 1) {
		std::string character_name = sep->arg[1];
		auto character_id = database.GetCharacterID(character_name);
		if (!character_id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Character '{}' does not exist.",
					character_name
				).c_str()
			);
			return;
		}

		Client *s = entity_list.GetClientByName(character_name.c_str());
		if (s) {
			t = s->CastToMob();
		} else {
			if (!worldserver.Connected()) {
				c->Message(Chat::White, "World server is currently disconnected.");
				return;
			}

			auto pack = new ServerPacket(ServerOP_ZonePlayer, sizeof(ServerZonePlayer_Struct));
			auto szp = (ServerZonePlayer_Struct *) pack->pBuffer;

			strn0cpy(szp->adminname, c->GetName(), sizeof(szp->adminname));
			strn0cpy(szp->name, character_name.c_str(), sizeof(szp->name));
			strn0cpy(szp->zone, zone->GetShortName(), sizeof(szp->zone));

			szp->adminrank          = c->Admin();
			szp->ignorerestrictions = 2;
			szp->instance_id        = zone->GetInstanceID();
			szp->x_pos              = c->GetX();
			szp->y_pos              = c->GetY();
			szp->z_pos              = c->GetZ();

			worldserver.SendPacket(pack);
			safe_delete(pack);
			return;
		}
	} else if (c->GetTarget()) {
		t = c->GetTarget();
	}

	if (c == t) {
		c->Message(Chat::White, "You cannot summon yourself.");
		return;
	}

	if (!t) {
		c->Message(Chat::White, "You must have a target to summon.");
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Summoning {} to {:.2f}, {:.2f}, {:.2f} in {} ({}).",
			c->GetTargetDescription(t),
			c->GetX(),
			c->GetY(),
			c->GetZ(),
			zone->GetLongName(),
			zone->GetZoneID()
		).c_str()
	);

	if (t->IsClient()) {
		t->CastToClient()->MovePC(
			zone->GetZoneID(),
			zone->GetInstanceID(),
			c->GetX(),
			c->GetY(),
			c->GetZ(),
			c->GetHeading(),
			2,
			c->GetHideMe() ? GMHiddenSummon : GMSummon
		);
		return;
	}

	t->GMMove(c->GetPosition());
}
