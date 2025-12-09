#include "../../client.h"
#include "../../worldserver.h"

extern WorldServer worldserver;

void SetOOCMute(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set ooc_mute [on|off]");
		return;
	}

	const bool is_muted = Strings::ToBool(sep->arg[2]);

	ServerPacket pack(ServerOP_OOCMute, sizeof(ServerOOCMute_Struct));

	auto o = (ServerOOCMute_Struct*) pack.pBuffer;
	o->is_muted = is_muted;

	worldserver.SendPacket(&pack);

	c->Message(
		Chat::White,
		fmt::format(
			"Server OOC is {} muted.",
			is_muted ? "now" : "no longer"
		).c_str()
	);
}
