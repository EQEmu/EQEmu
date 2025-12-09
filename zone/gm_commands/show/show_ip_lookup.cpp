#include "../../client.h"
#include "../../worldserver.h"

extern WorldServer worldserver;

void ShowIPLookup(Client *c, const Seperator *sep)
{
	const uint32 ip_length = strlen(sep->argplus[2]);

	auto pack = new ServerPacket(
		ServerOP_IPLookup,
		sizeof(ServerGenericWorldQuery_Struct) + ip_length
	);

	auto s = (ServerGenericWorldQuery_Struct *) pack->pBuffer;
	strn0cpy(s->from, c->GetName(), sizeof(s->from));
	s->admin = c->Admin();

	if (ip_length) {
		strcpy(s->query, sep->argplus[2]);
	}

	worldserver.SendPacket(pack);
	safe_delete(pack);
}
