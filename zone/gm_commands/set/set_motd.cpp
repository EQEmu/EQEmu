#include "../../client.h"
#include "../../worldserver.h"

extern WorldServer worldserver;

void SetMOTD(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set motd [Message]");
		return;
	}

	const std::string& message = sep->argplus[2];

	auto pack = new ServerPacket(ServerOP_Motd, sizeof(ServerMotd_Struct));

	auto m = (ServerMotd_Struct *) pack->pBuffer;
	strn0cpy(m->myname, c->GetName(), sizeof(m->myname));
	strn0cpy(m->motd, !message.empty() ? message.c_str() : "", sizeof(m->motd));

	worldserver.SendPacket(pack);
	safe_delete(pack);
}
