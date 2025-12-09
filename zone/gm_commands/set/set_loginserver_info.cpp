#include "../../client.h"
#include "../../worldserver.h"

extern WorldServer worldserver;

void SetLoginserverInfo(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 3) {
		c->Message(Chat::White, "Usage: #set lsinfo [Email] [Password]");
		return;
	}

	const std::string& email    = sep->arg[2];
	const std::string& password = sep->arg[3];

	auto pack = new ServerPacket(ServerOP_LSAccountUpdate, sizeof(LoginserverAccountUpdate));

	auto s = (LoginserverAccountUpdate *) pack->pBuffer;

	s->user_account_id = c->LSAccountID();
	strn0cpy(s->user_account_name, c->AccountName(), 30);
	strn0cpy(s->user_email, email.c_str(), 100);
	strn0cpy(s->user_account_password, password.c_str(), 50);

	worldserver.SendPacket(pack);
	safe_delete(pack);

	c->Message(Chat::White, "Your email and local loginserver password have been set.");
}
