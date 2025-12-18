#include "zone/client.h"
#include "zone/worldserver.h"

extern WorldServer worldserver;

void command_kick(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #kick [Character Name]");
		return;
	}
	
	std::string character_name = sep->arg[1];
	auto client = entity_list.GetClientByName(character_name.c_str());
	if (client) {
		if (client->Admin() <= c->Admin()) {
			auto outapp = new EQApplicationPacket(OP_GMKick, 0);
			client->QueuePacket(outapp);
			safe_delete(outapp);
			client->Kick("Ordered kicked by command");
			c->Message(
				Chat::White,
				fmt::format(
					"{} has been kicked from the server.",
					character_name
				).c_str()
			);
		}
	} else if (!worldserver.Connected()) {
		c->Message(Chat::White, "The world server is currently disconnected.");
	} else {
		auto pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
		ServerKickPlayer_Struct *skp = (ServerKickPlayer_Struct *) pack->pBuffer;
		strcpy(skp->adminname, c->GetName());
		strcpy(skp->name, character_name.c_str());
		skp->adminrank = c->Admin();
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

