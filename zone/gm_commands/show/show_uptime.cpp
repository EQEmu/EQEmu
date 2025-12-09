#include "../../client.h"
#include "../../worldserver.h"

extern WorldServer worldserver;

void ShowUptime(Client *c, const Seperator *sep)
{
	auto pack = new ServerPacket(ServerOP_Uptime, sizeof(ServerUptime_Struct));

	auto s = (ServerUptime_Struct *) pack->pBuffer;
	strn0cpy(s->adminname, c->GetName(), sizeof(s->adminname));

	if (sep->IsNumber(2) && Strings::ToUnsignedInt(sep->arg[2]) > 0) {
		s->zoneserverid = Strings::ToUnsignedInt(sep->arg[2]);
	}

	worldserver.SendPacket(pack);
	safe_delete(pack);
}
