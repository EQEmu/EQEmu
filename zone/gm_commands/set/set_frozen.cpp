#include "../../client.h"

void SetFrozen(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set frozen [on|off]");
		return;
	}

	if (!c->GetTarget()) {
		c->Message(Chat::White, "You must have a target to use this command.");
		return;
	}

	const bool is_frozen = Strings::ToBool(sep->arg[2]);

	auto t = c->GetTarget();
	if (c == t) {
		c->Message(Chat::White, "You cannot use this command on yourself.");
		return;
	}

	t->SendAppearancePacket(AppearanceType::Animation, is_frozen ? Animation::Freeze : Animation::Standing);

	c->Message(
		Chat::White,
		fmt::format(
			"You have {}frozen {}.",
			!is_frozen ? "un" : "",
			c->GetTargetDescription(t)
		).c_str()
	);
}
