#include "zone/client.h"

void command_movechar(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #movechar [Character ID|Character Name] [Zone ID|Zone Short Name]");
		return;
	}

	const std::string &character_name = (
		sep->IsNumber(1) ?
		database.GetCharNameByID(Strings::ToUnsignedInt(sep->arg[1])) :
		sep->arg[1]
	);
	const uint32 character_id = database.GetCharacterID(character_name);
	if (!character_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Character {} could not be found.",
				character_name
			).c_str()
		);
		return;
	}

	const uint32 account_id = database.GetAccountIDByChar(character_name.c_str());

	const std::string &zone_short_name = Strings::ToLower(
		sep->IsNumber(2) ?
		ZoneName(Strings::ToUnsignedInt(sep->arg[2]), true) :
		sep->arg[2]
	);

	const bool is_unknown_zone = zone_short_name.find("unknown") != std::string::npos;
	if (is_unknown_zone) {
		c->Message(
			Chat::White,
			fmt::format(
				"Zone ID {} could not be found.",
				Strings::ToUnsignedInt(sep->arg[2])
			).c_str()
		);
		return;
	}

	const uint32 zone_id = ZoneID(zone_short_name);
	auto         z       = GetZone(zone_id);

	if (!z) {
		c->Message(Chat::Red, "Invalid zone.");
		return;
	}

	const bool  moved        = database.MoveCharacterToZone(character_name, zone_id);
	std::string moved_string = moved ? "Succeeded" : "Failed";
	c->Message(
		Chat::White,
		fmt::format(
			"Character Move {} | Character: {} ({})",
			moved_string,
			character_name,
			character_id
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Character Move {} | Zone: {} ({}) ID: {}",
			moved_string,
			z->long_name,
			zone_short_name,
			zone_id
		).c_str()
	);
}
