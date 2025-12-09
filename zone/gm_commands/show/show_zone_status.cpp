#include "../../client.h"
#include "../../worldserver.h"

extern WorldServer worldserver;

void ShowZoneStatus(Client *c, const Seperator *sep)
{
	auto pack = new ServerPacket(ServerOP_ZoneStatus, sizeof(ServerZoneStatus_Struct));

	auto z = (ServerZoneStatus_Struct *) pack->pBuffer;
	z->admin = c->Admin();
	strn0cpy(z->name, c->GetName(), sizeof(z->name));

	worldserver.SendPacket(pack);
	delete pack;
}
