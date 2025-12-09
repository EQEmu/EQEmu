#include "../../client.h"
#include "../../worldserver.h"

extern WorldServer worldserver;

void ShowClientVersionSummary(Client *c, const Seperator *sep)
{
	auto pack = new ServerPacket(ServerOP_ClientVersionSummary, sizeof(ServerRequestClientVersionSummary_Struct));

	auto s = (ServerRequestClientVersionSummary_Struct *) pack->pBuffer;
	strn0cpy(s->Name, c->GetName(), sizeof(s->Name));

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

