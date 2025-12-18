#include "zone/client.h"

void command_resetaa(Client *c, const Seperator *sep)
{
	Client* t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint16 arguments     = sep->argnum;
	const bool   is_aa         = !strcasecmp(sep->arg[1], "aa");
	const bool   is_leadership = !strcasecmp(sep->arg[1], "leadership");

	if (!arguments || (!is_aa && !is_leadership)) {
		c->Message(Chat::White, "Usage: #resetaa aa - Resets and refunds a player's AAs");
		c->Message(Chat::White, "Usage: #resetaa leadership - Resets a player's Leadership AAs");
		return;
	}

	if (is_aa) {
		t->ResetAA();
	} else if (is_leadership) {
		t->ResetLeadershipAA();
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Successfully reset all{} AAs for {}.",
			is_aa ? "" : " Leadership",
			c->GetTargetDescription(t)
		).c_str()
	);
}
