#include "../../client.h"

void SetHideMe(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set hide_me [on|off]");
		return;
	}

	auto t = c;
	if (c->GetGM() && c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const bool is_hidden = Strings::ToBool(sep->arg[2]);

	t->SetHideMe(is_hidden);

	c->Message(
		Chat::White,
		fmt::format(
			"{} {} now {}visible to players below a status level of {}.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "are" : "is",
			is_hidden ? "in" : "",
			t->Admin()
		).c_str()
	);
}
