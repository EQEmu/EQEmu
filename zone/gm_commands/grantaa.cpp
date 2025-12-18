#include "zone/client.h"

void command_grantaa(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsClient()) {
		c->Message(Chat::White, "You must target a player to use this command.");
		return;
	}

	const uint8 unlock_level    = sep->IsNumber(1) ? static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[1])) : 0;
	const bool  skip_grant_only = sep->IsNumber(2) ? Strings::ToBool(sep->arg[2]) : false;

	auto t = c->GetTarget()->CastToClient();
	t->GrantAllAAPoints(unlock_level, skip_grant_only);

	c->Message(
		Chat::White,
		fmt::format(
			"Successfully granted all Alternate Advancements for {}{}{}.",
			c->GetTargetDescription(t),
			(
				unlock_level ?
				fmt::format(
					" up to level {}",
					unlock_level
				) :
				""
			),
			skip_grant_only ? "except for grant only AAs" : ""
		).c_str()
	);
}
