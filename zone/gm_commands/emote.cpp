#include "zone/client.h"
#include "zone/worldserver.h"

extern WorldServer worldserver;

void command_emote(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (arguments < 3 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #emote [Name] [Type] [Message]");
		c->Message(Chat::White, "Usage: #emote world [Type] [Message]");
		c->Message(Chat::White, "Usage: #emote zone [Type] [Message]");
		return;
	}

	if (!worldserver.Connected()) {
		c->Message(Chat::White, "Error: World server disconnected!");
		return;
	}

	bool is_world = !strcasecmp(sep->arg[1], "world");
	bool is_zone = !strcasecmp(sep->arg[1], "zone");

	const std::string emote_message = sep->argplus[3];
	const auto emote_type = Strings::ToUnsignedInt(sep->arg[2]);

	if (is_zone) {
		if (!Strings::Contains(emote_message, "^")) {
			entity_list.Message(0, emote_type, emote_message.c_str());
			return;
		}

		for (const auto& m : Strings::Split(emote_message, "^")) {
			entity_list.Message(0, emote_type, m.c_str());
		}
	} else {
		if (!Strings::Contains(emote_message, "^")) {
			worldserver.SendEmoteMessage(
				is_world ? 0 : sep->arg[1],
				0,
				emote_type,
				emote_message.c_str()
			);
			return;
		}

		for (const auto& m : Strings::Split(emote_message, "^")) {
			worldserver.SendEmoteMessage(
				is_world ? 0 : sep->arg[1],
				0,
				emote_type,
				m.c_str()
			);
 		}
	}
}

