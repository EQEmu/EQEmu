#include "zone/client.h"
#include "zone/worldserver.h"

extern WorldServer worldserver;

void command_zonebootup(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #zonebootup [Zone Server ID] [Zone ID|Zone Short Name] [Instance ID]");
		return;
	}

	if (!worldserver.Connected()) {
		c->Message(Chat::White, "World server disconnected.");
		return;
	}

	const uint32 zone_server_id = Strings::ToUnsignedInt(sep->arg[1]);
	const uint32 zone_id        = Strings::IsNumber(sep->arg[2]) ? Strings::ToUnsignedInt(sep->arg[2]) : ZoneID(sep->arg[2]);
	const bool   is_static      = arguments > 2 ? !strcasecmp(sep->arg[3], "static") : false;
	const uint16 instance_id    = arguments > 3 ? static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[4])) : 0;

	if (!zone_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Zone '{}' does not exist.",
				sep->arg[1]
			).c_str()
		);
		return;
	}

	auto pack = new ServerPacket(ServerOP_ZoneBootup, sizeof(ServerZoneStateChange_Struct));
	auto *s   = (ServerZoneStateChange_Struct *) pack->pBuffer;

	s->zone_server_id = zone_server_id;
	s->zone_id        = zone_id;
	s->instance_id    = instance_id;
	s->is_static      = is_static;

	strn0cpy(s->admin_name, c->GetName(), sizeof(s->admin_name));

	worldserver.SendPacket(pack);
	safe_delete(pack);
}
